# fnotify
我的个人博客是用jekyll来做的，jekyll的主要工作是根据用户个人的模板把对应的博文build成html，然后自己将html发布出来，这样，个人的博客就搭建起来了。  
所以jekyll非常轻量级非常好用，但是jekyll有个缺点，每当我写好博文上传到服务器，就得用jekyll去build一次，才能发布出来，我觉得太麻烦了。  
要想不麻烦就得需要个守护进程，既能感知我新增了博文，删除了博文，更新了博文，又能在感知后自动build。  
应付这个问题，可以采用inotify + select来解决。inotify能对文件系统上的一切操作做出回应，而select可以进行io的多路复用。

# Example Usage 
* build  
    ```ini
    $ cd /root/fnotify
    $ make
    ```
* conf

    ```ini
    $ cat /root/fnotify/fnotify.conf
    [fnotify]
    path=/root/jekyll-zdt/_posts
    cmd=jekyll build --source /root/jekyll-zdt
    delay=3
    
    [fnotify]
    path=/root/jekyll-gg/_posts
    cmd=jekyll build --source /root/jekyll-gg
    delay=3
    ```
* execute  
*because of deamon(0, 0),please set the conf's absolute path to the argument*
    ```ini
    $ fnotify /root/fnotify/fnotify.conf
    ```
