# ECE568 HW2
The cache size is 50, using FIFO strategy. We handle the following HTTP `Cache-Control` header fields:
1. no-store
2. no-cache
3. max-age
4. max-stale
5. min-fresh
6. private
7. no `Cache-Control` in neither request and response

When receiving a new request, if request can be found in cache but requires re-validation, the log follows the following strategy:
1. first 
## Test GET
### case 1:
web url: http://people.duke.edu/~bmr23/ece568/class.html

建议使用下列网址：

http://pfister.ee.duke.edu/index.html

http://people.duke.edu/~bmr23/ece568/class.html

## 测试CONNECT
注意CONNECT有死循环， 运行一次只能开一个https

可以用来测试的网站：

https://www.google.com

https://www.baidu.com
## 编译
C++11

我一直用cmake， 你要用make的话需要自己写Makefile。建议关闭编译器优化（我一直是关闭的）
## 已知BUG
程序不稳定，另外注意浏览器缓存和试探链接的影响。
## 你的任务
你**必须**在26号完成POST以及多线程。这两部分真的没多少要写， 主要是把我现有的代码进行整理， 估计要新写的不会超过50行。我已经写了300多行， 你要写的真的不多。26号我们开始做cache和log。时间非常紧张。
