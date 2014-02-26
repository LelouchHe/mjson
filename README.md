
## region 你们看不到

这篇文档是开发mjson的思路的汇总,没有做任何整理,基本是按时间顺序來的

所以可以看到我思路确实不大好啊

退散...

## 原文开始

使用了很长时间的json作为通信协议,觉得可以尝试编写json的解析库,相信是个不错的练习

## 总的要求:
* 内存消耗一定要小
* 速度要够快
* 提供的api要方便使用
* 使用纯C來完成(锻炼自己啦)

目前还没有什么好的思路,暂时把api补全了

## 实现的一点思路
* 不得不解析时再解析.我自己使用json的经验是,很多时候只使用了json的一小部分内容,其他的内容其实大可不必解析,以节约时间和内存消耗
* 共享不可变的原生字符串和COW.这样就不需要每个节点都保存完整的字符串.可能需要带有引用计数的字符串来保存
* 内存的分配有root连接起来,最后统一释放.使用过程中,除了root之外,并不需要用户释放其他节点(当然,除非又new/copy了一个json).想过把json也做成引用计数,但这样使用起来难度过大,故放弃
* iterator应该还是要暴露结构的,否则很难做到局部变量,保持使用习惯

## 关于COW

写的时候重新申请内存放置新的内容,那么什么时候回收呢?当我们需要mj_write或者mj_buf_size時,往往需要遍历所有已经生成的json串:如果被修改过,肯定得重新生成字符串格式,如果没有,就直接返回保存的原始字符串.重新生成之后,是否需要再把节点下的所有字符串拼接起来呢?

我觉得是有必要的,因为2个调用,最后都是要生成字符串的,这样更加节约内存.我们可以保留已经解析成功的json不动,仅仅把字符串表示归一一下,这样也不会影响以后解析的速度.

唯一的缺点是,可能会频繁的申请/释放内存,这个感觉暂时可以接受,实在不行,可以改变共享字符串的实现,底层使用ring來保存常见字符串即可,而依赖于其上的json处理不变

## map的实现

现在实现了一个小型的hashmap,用以保存json中的object对象的kv.此处,kv的生存期由调用方保证,k使用了双hash來简化判断

此处,k比较微妙.最好的方式是k从原生字符串來,因为k基本不会变化,所以是常量.但这样的话,k就不是单纯的char *,而是复杂的引用计数字符串了.

所幸,原有的逻辑应该不会太大变动,等实现了引用计数字符串,再來修改map

## ref_str的实现

做这个很纠结,因为感觉不论怎样定义接口,使用起来总是没有原生的**char**來的舒坦,不过聊胜于无,姑且先利用这个版本的ref_str吧

这个版本的ref_str倾向于使用局部变量的形式嵌套进其他结构体或函数体内.因为如果以指针形式引入,势必会带来生存期的问题,谁來管理这个指针,而且这里又需要动态分配内存了(其实一样,区别在于调用1次malloc或者2次)

局部变量的问题就是需要暴露很多内部信息,这里暴露的begin/end和内部的指针

我觉得还是改回来吧

## 再论COW

随着实现的深入,觉得COW可能不是很好的方式.比如每次as_string时,虽然我们不需要更改该json结构,但是呢,返回的char*指向什么位置呢?我们最好可以返回的是常规的字符串(以'\0'结尾),所以ref_str可能不是很好的选择,所以此时应该也需要新建一个ref_str保留这个字符串

所以看来,不仅写时需要新建,在提取字符串时同样需要.

在返回其他类型时则不需要类似的,因为字符串是最常见的转型,而且人们对字符串转型应该是可以预期的,而其他的不然

## json结构

基础的json大致需要以下几个字段:
* type: 类型,共8个,用3bit即可
* is_root: 是否是根节点.当删除节点時,本节点是否一同释放,是个问题,对于根节点,是可以的,但子节点,是不能删除的(否则其父节点指针就over了),所以,对于子节点,应该把它的子节点全部删除,但保留本身(设置为NULL节点即可)
* is_dirty: 是否被改写过,改写之前需要确认是否相同再修改.主要用于递归write時
* is_str: text是否为char*.当as_string時,肯定需要一个'\0'结尾的字符串,这个的生存期肯定也得受json控制,那么此时就直接复用text.不过这个只针对基础类型(非object和array),因为基础类型的text,即使是ref_str,也不会有人來引用的

以上三个总共占6bit,再剩余2bit备用

### 更新

既然我们已经采用引用计数了,那么是否还有必要保留is_root呢?

我觉得没有必要了,is_root是一个临时的产物,当引用计数为0时,必然可以删除这个节点的.

引用计数主要不是为了避免内存泄露(引用计数实际比内存更恶心有没有),主要是为了不让在使用json时,发现json底层已经没了,然后就core

所以使用时,首先要增加ref,使用完毕,手动deref,或者加入其他组合类型中,自动deref

## 关于表示

因为要处理删除甚至转型,所以直接操作引用json结构貌似不是很可行(就像mjson一度设计的那样,类似继承的包含结构,其实也是可以的,比如改用union)

现在的表示就是二级结构,操作引用的是一个类似句柄的东东,里面再指向真正的表示,这样的表达就更加灵活的

添加句柄的好处是增加了一个间接层,很多额外的东西我们可以操作了,比如引用计数,比如统计,再比如我们可以任意赋值,而不用担心指针问题(因为句柄不变,只是指向变了而已).坏处是增加了内存消耗,不过此处额外的只有ref,所以应该问题不大

## 字符串表示总结

* bool/null属于静态类型,指向的是全局静态变量,ref_str指向的是const char*(is_str===1)
* int/double/str属于基本类型,不会进行初始化,但as_string时会有默认值(如果为空的话).is_str标识ref_str是否为char*
* object/array属于组合类型,永远使用ref_str(is_str===0)

这样的分类主要是基于后续的使用,一来初始化后没有赋值直接使用本来很少见,所以选择不进行初始化,二来,组合类型的子节点可能复用父节点的ref_str,但基本类型则很少用到了

## 引用计数

以前觉得引用计数很难,实际实现之后发现有那么多的模型需要折衷,越发觉得难了

现在我能想到的有2种基本模型:
* refp -> ref+data: 所有的使用都是基于同一个refp的.这样的优点是不需要每次都动态生成新的refp,缺点就是没有办法阻止我们不停的dec这个refp,只要refp指向还有效,就可以一直错误的dec,直到被销毁.这其实没有什么,关键是没有办法检测到类似的情况
* refpa -> ref+data <- refpb: 每次使用都是新的refp,ref不再是真正使用者,而是refp的数量了(第一种refp不变,ref记录的是真正使用者的数量).这个缺点就是上面那个的优点(动态生成refp),优点就是我们有方法阻止不停的dec了,直接把refp释放掉,如果2次dec,就是AV,直接可能会core

还有个问题是随之而来的,即试图修改refp会带来什么样的后果,比如reset.我们是reset这个refp(比如调整其指向),还是修改data的值.而且使用2种不同的模型,也会带来不同的含义(一个是一变全变,另一个是只有一个变)

### 使用哪种模型和reset语义

在mjson中,我们的目标是尽量做到动态语义.比如mj的类型本来是MJSON_OBJECT,但是我可以直接赋值为array,mj的类型就变成了MJSON_ARRAY

同样有2种模型:
* p -> union: 这样p不需要变动,直接操作union中的值即可,里面可以是类型之类的json结构
* p -> handle -> value: 这样value是独立的value,修改p,实际上是修改handle的指向,并不影响原先的value

union是占空间较大,handle的缺点是handle/value都需要动态生成(union的value本身不需要,只需要赋值即可)

针对mjson,其实二者相差不多,union也就多一个指针大小,而handle也多出指针大小

### 混淆

感觉我在这里混淆了2个概念--引用指针的reset和json的reset.而且json的设计动不动就扯上了refp上.

看来选择太多,让我都不能前进了.好吧,理清思路

### 最后的表示

经过很长时间思考和实践,决定了mjson_t的表示就是refp,即我们操作的是指向mjson_value_t的引用指针,这样感觉是最好的决定了

当然,对外暴露的结构是void*,内部会进行相应的转换

## 类型赋值取值

再来实现针对特定类型的赋值取值,策略按照不同类型分为3类
* 静态类型: bool/null,基本通过类型就能获取值
* 基本类型: str/int/double,可能尚未解析(值为默认值时,可能没有解析),没有解析的话,text可能为ref_str或者char*(需要根据is_str判断).一旦解析,就把ref_str转为char*
* 组合类型: 永远是ref_str,可能尚未解析(为默认值,就是NULL),所以需要解析,check,再取值赋值

ps: 觉得is_str貌似有些不是很好,要不干脆基本类型也使用ref_str吧,也就多2个int大小而已.这个想法暂时保留

还需要注意的一个问题是,取值时是否需要保留住当前的json.有可能要被别人fini,这样就不可能成功了(铁定AV了).所以,可能还需要一个基本操作就是lock/unlock(我们不提供直接inc/dec,这个名字太明显了,而lock/unlock一般人们不会忍不住不停的lock的,这样就**死锁**了)

### 接口

现在设计的接口是利用返回值来表示get,但是这里有一个问题,如果出错了,返回什么值?这个值除了str的NULL之外,其实都是合法的值,使用者如何判断一个值是非法的,还是恰好是非法的合法值呢?

其实可以这样,在转换接口的参数中加入判断错误的结构,如果用户不关心结果,就可以直接传NULL,那么用户需要自己解决这个判断问题(我们还可以封装一个接口来简化调用),否则,用户就可以通过这个结构来判断

这个结构应该可以是局部变量的那种完全暴露的数据,具体的值还没有想好,不过先这样做着看

## 再次总结下目前的设计

* 所有的set都返回原来的mjson_t指针,提供引用语义.如果需要值语义,提供mj_copy,此时需要手动fini才可以
* set接口分2种:0.特定类型:检查类型,并直接修改;1.统一类型(mjson_t):类型相同且是唯一指针,则直接修改,否则新建值并reset
* get接口同样,不同的是get_str接口是每个类型都必须提供的
* get/set接口最好还是采用mjson_value_t参数和返回值.其实,这里做的,很类似把mjson_value_t作为基类,其他都是派生类,统一接口提供了类似虚函数的作用
* 所有以mjson/MJSON开头的,都是内部私有的,MJ/mj开头的应该都是公共的.不过文件内部自有的,可以随意,这个规则处理的是头文件里的

## parse int/double

现在开始parse部分,按照延迟处理的原则,每个节点在使用时才会解析,在更改/as_string才会更改,所以parse需要应对很多底层表示

现在的parse不能直接使用系统的解析,因为它不一定以'\0'结尾,所以需要手写

我发现最好的手写方式就是状态机,只要列清楚,把图画好,肯定是很快实现的.唯一的缺点是,状态机可能有太多条件判断,效率有些影响

这个容我再看看其他实现再说,反正这个只是parser模块的一部分,不影响其他

## parser存在的意义

有的时候想,我们在赋值ref_str时,肯定会遍历一遍对应字符串的,为什么不在这个时候进行解析呢?反而是要在后来,再解析一遍呢?

我的想法是这样的:
* 遍历一遍字符串,查找某项的开始和结束,是比较简单的.比如object是要找配对的'{}'即可,array是'[]',字符串是'""',其他是数字或固定字符串.遍历还是简单的,相反,解析,是耗时的,int/double还好说,组合类型比较耗时,所以我们压后,我们前期需要的只是类型和边界而已,这个一般只需要判断首个字符即可
* 现在的parser只提供一个功能,即给定begin/end,验证并返回对应值.还会提供一个功能,返回下一个完整字符串上的类型和起始,这样,我们统一使用parser即可了

## parser的使用

怎么使用这个parser,是目前的难点,因为在没有深入了解内部结构之前,确实很难不解析内容直接判断结构的边界

### 使用parser的场景

通常我们这样来使用json解析

    const char *data = "[1, 2, 3]";
    size_t len = strlen(data);
    mjson_t *mj = mj_parse(data, len);
    size_t n = mj_size(mj);
    for (size_t i = 0; i < n; i++) {
        mjson_t *v = mj_get_iv(mj, i);
        printf("%d\n", mj_get_int(v));
    }

使用方会提供字符串的边界(data, len),我们可以预先假设提供的是正确的.

### 解析

一开始的时候,可以不解析,先把data保存在text中,根据首字符'['判断出这是个array类型的,新建mjson_array_t来保存这个结构,其他的都为空

当mj_size时,其实同样不需要解析整个array,相反,我们可以通过','来判断元素的个数,这样的算法应该更快(需要考虑字符串和其他内嵌结构中的','的影响)

当mj_get_iv时,这个时候我们需要进行解析了,i==0时,只解析第一个元素(1),判断是INTEGER,新建mjson_int_t节点,保存指向这个的ref_str,并返回即可

此处有个问题,在解析i==1时,我们如何下手?还是从0开始,跳过第一个',',还是保存某些中间量,告知目前我们解析了哪些地方呢?

中间量的方法不是很行,如果不是非顺序访问,可能会造成很大的空间开销.我这里的方法是只要访问一个元素,就把所有元素都解析出来,但仅仅是判断类型,设置好ref_str.比如此处,访问i==0时,就把3个节点都新建好,插入到vec中(但是不解析这些int节点),当访问时,我们就可以直接访问了

    /* 伪码如下 */
    mjson_t *mj_get_iv(mj, i) {
        if (mj->v == NULL) {
            /* 全部解析开,但只限于第一层 */
            mj->v = mp_array(mj->ref_str);
        }
        return mj->v[i];
    }

这样如果不访问这个array(get),那么这个array永远不会展开解析的,我感觉这样可能更好

## rp_ini失败的处理

在很多带is_move参数的函数中,失败后总会把move的东西析构,这个功能很好.

同样的功能在rp_ini也是需要的(但目前还没有).我们是否要改变rp_ini的行为呢?

个人认为是否定的

这种违背原始语义的行为,应该通过更明显的方式表达,比如改变函数签名,而不是在背后偷偷修改

所以,如果要修改的话,要新建函数,加入is_move参数.但目前还不想这么做

## mj_write的处理

一直没有处理str类型的get/set,set其实好说,同其他类型一致,但get有些麻烦.我理想中的get应该和parse是相反的动作,是每个json都可以进行的操作,返回对应的字符串,所以这个叫做write,幷单开了一个文件表示

如何处理呢?

### 相关字段含义

* mv.text == NULL: 采用默认初始化(mjson_ini)
                   组合类型默认初始化后,进行set/get,且没有write之前,同样是不会设置text的,这点和其他类型有明显差别
* mv.is_str: 组合类型永远为0,静态类型永远为1,基本类型都有可能
* mv.is_dirty: set之后会设置(write之后,如果ref为1,则清空,否则不清)
               对于组合类型,get不存在的值,同样也会设置(幷默认添加MJSON_NULL节点)

### 一些定义

p.dirty = p.is_dirty || children.is_dirty

父节点的dirty是由本身和子节点定义的,叶节点的dirty之和自身有关

### dirty的取法

现在看来,采用dfs遍历整个json树是比较可行的,一旦某个节点is_dirty了,那么之上的所有节点都一样的

不dirty的节点可以直接输出text中的内容(除了默认初始化的元素,这个时候需要赋初值)

dirty的节点一方面需要把子节点的表达规整,另一方面,也要更新自己的text,幷按照规则,清空dirty位,加快下一次write的速度

### 如何输出

分成2步
1. dfs遍历,一是查看dirty节点,二是计算最后的缓冲区大小,便于输出
2. 再次dfs遍历,这次拿着buf遍历,把所有的节点都输出上来

第2步時,是否需要调整节点的text呢?把过去的释放,重新附在新的text上?

我觉得不需要了,这样频繁的malloc/free对性能不好,而且很容易某个地方出错,造成内存问题,再说,节点指向原先的text,说明本身不dirty,不需要操作,指向自己的text,说明曾经访问过,那么就可能再次访问,到时候还是要再次申请,岂不麻烦

### 再论dirty

现在看来,is_dirty这个字段十分的无用,基本类型/静态类型是每次必算,组合类型由于不知道子节点的情况,所以每次还是必须得遍历一边,那么这个状态还有什么意义呢?

考虑在下一次重构中删去

## array/object的get

现在object的语义类似与map结构,get的时候如果该key不存在,那么就会新建一个value与之对应,类型为MJSON_NULL

但array中却还没有最终决定下来,底层的vector是如此的语义:
* offset超过了num,直接返回NULL,vector不自动增长
* offset没有超过num,返回对应指针,所有指针都会初始化为NULL,直到被赋值

对于底层而言,NULL有2个含义,一个是超过边界的NULL,另一个是默认值NULL.

我们的array要采用哪种语义呢?是不存在的情况给个MJSON_NULL的默认值,还是不存在直接NULL就好了?如果使用前者,就必须确定vector返回的2种NULL的不同,一种还是为NULL,另一种返回MJOSN_NULL

个人倾向于区别于object,对于不存在的直接返回NULL即可,在表达上支持类似[1,2,,3]的表示,其中offset为2的就是不存在的NULL

其实想一下,object同样也可以这样,比如{"a":, "b":123},其中a对应的就是NULL.

### 更新

现在想想,感觉其实不是太好,支持类似空余字段的感觉很丑陋,所以干脆只要不超过边界,都返回MJSON_NULL类型了.

不过用点trick:
* 初始化時还是NULL,只有get時才进行初始化操作(MJSON_NULL的初始化只有refp的分配而已)
* 现在vec置NULL是合法的set,删除必须使用erase.这个同map的set不一样,vec需要预分配空间,最合理的占位符就是NULL,没道理不能用,而map随用随占,所以没这个顾虑

## is_dirty的去留

现在的writer没有使用到is_dirty,对于组合类型,是以m/v是否初始化作为是否修改的依据.这样的情况处理比较简单,但性能影响应该也是有的

比如只有get操作,且get有key的对象,那么虽然最后m肯定是要被初始化的,但不影响字符串的表示

在计算字符串长度,预备缓冲区的遍历过程中,应该可以同时计算出对应的is_dirty,所以加上只有好处没有坏处

随之而来的问题是,is_dirty什么时候清0?
