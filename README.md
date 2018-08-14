# libpump

<div align=center>
<img src="https://github.com/csjy309450/libpump/blob/master/resource/pump.png?raw=true"/></div>

轻量级的跨平台事件循环库框架，采用 (Wather) 插件的形式监听用户注册 (Event) 事件.

## 0. requirement pkgs

- gcc ( >= 5.4.0 ) 
  使用c98标准
- sgistl 
  ( gcc默认使用sgistl, windows平台可以下载编译stlport, 也可以直接使用vc的stl库 )
- boost ( 1.56.0 )
- glog 
- protobuf
  - benchmark
  - googletest
- cmake ( >= VERSION 3.5 )

## 1. compile

```shell
cd LIBPUMP_ROOT
cmake CMakeLists.txt
make -j8
```



