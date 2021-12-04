# MEAMVC
This is the source code of MEAMVC.

MEAMVC的源代码。

If you feel our work helpful, please cite:
```
@article{guo_meamvc_2019,
	title = {{MEAMVC}: {A} {Membrane} {Evolutionary} {Algorithm} for {Solving} {Minimum} {Vertex} {Cover} {Problem}},
	volume = {7},
	doi = {10.1109/ACCESS.2019.2915550},
	journal = {IEEE Access},
	author = {Guo, Ping and Quan, Changsheng and Chen, Haizhu},
	year = {2019},
	pages = {60774--60784},
}
```

# Run
Use 'make' to compile the source code, or import it to Qt or other ides and compile.

① 安装Qt（可以从http://download.qt.io/archive/qt/5.14/5.14.1/ 根据你的操作系统下载），安装完成之后双击MEAVC.pro打开项目，指定运行参数之后（项目界面，参数示例：D:/Data/bio-dmela--2630.mtx.mis 1 10 5 6 10 50），点击绿色小三角运行。Qt的用法可以百度相关教程。

② 使用命令行make指令进行编译源代码，编译完成之后在命令行调用编译之后的程序运行（命令行运行示例：./meavc D:/Data/bio-dmela--2630.mtx.mis 1 10 5 6 10 50）。make指令，如果是win10系统，并不自带make指令，需要安装linux子系统，在linux子系统下进行编译。如果是linux系统，可以直接在解压目录下运行make指令和编译后的程序。

③ 使用其他的ide，导入源代码后编译运行，例如visual studio等。
