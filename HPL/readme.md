# HPL

> 本题难度较大，对linux系统知识与C项目编译经验要求较高。
> 做题过程中可能会遇到大量你陌生的知识，请学会使用搜索引擎与人工智能解决

## 简要介绍

HPL（The High-Performance Linpack Benchmark）是测试高性能计算集群系统浮点性能的基准程序。HPL通过对高性能计算集群采用高斯消元法求解一元N次稠密线性代数方程组的测试，评价高性能计算集群的浮点计算能力，也是TOP500超级计算机排名的重要依据之一。

在最新的[TOP500榜单](https://www.top500.org/lists/top500/list/2024/06/)中可以查看到上榜的超级计算机的HPL性能($R_{max}$)。

## 你的任务

在比赛集群的单台机器上运行HPL测试。

你可以在`source_code`文件夹下运行`bash download_hp.sh`下载并解压得到hpl源码(将位于`source_code/hpl-2.3`文件夹下)。你需要先完成HPL的编译得到可执行文件xhpl，然后写一份slurm脚本提交给调度系统让你的xhpl运行在单台机器的128个核心上。同时，你可以通过调整HPL.dat的参数以获得更好的性能。

## 任务指导

### 编译运行

HPL源码中已经提供了Makefile，但是其编译命令为
```bash
make arch=<your arch>
```
其中\<your arch\>需要你自行指定，运行后将会调用同级目录下的Make.\<your arch\>配置文件进行编译。并将最终的可执行文件和默认的HPL.dat(xhpl运行时的配置文件)生成在bin/\<your arch\>/目录下。

但是很遗憾并没有Make.\<your arch\>文件可以直接用于编译，需要你自己写一份用于编译。

好在setup文件夹下已经预置了很多常见的架构下的配置文件，你可以选择一个将其复制到Makefile的同级目录下并修改名称为`Make.xflops`根据文件中的提示进行修改，修改完成之后使用命令
```bash
module load ... # 加载你可能需要使用的环境
make arch=xflops
```
进行编译。

编译完成之后请修改HPL.dat文件中的参数为合适的值，并写一份hpl.slurm脚本，要求其在Makefile同级目录下提交之后能够在单机128核心上运行HPL测试并将**所有**输出重定向到Makefile同级目录下的myhpl.out文件中。

### 性能调优

你可以从以下两个方面提高你的HPL性能:
* 修改Make.xlops文件，尝试使用更为快速库或编译器优化提升性能
* 修改HPL.dat文件，尝试通过调整参数得到更高的性能

## 评分标准

本题你需要提交**make_env.json, Make.xflops, HPL.dat, hpl.slurm, myhpl.out**五个文件。

* `make_env.json`：指定在编译时需要加载的环境。将你需要加载的环境写入键"modules"对应的值数组中，例如你需要`module load bisheng/2.5.0`则在数组中添加元素"bisheng/2.5.0"
* `Make.xflops`: 编译时使用的配置文件
* `HPL.dat`：运行HPL使用的配置文件
* `hpl.slurm`：运行HPL使用的sbatch提交脚本。请注意，我们将会在`Makefile`所在文件目录下运行`sbatch hpl.slurm`提交你的脚本进行测试。
* `myhpl.out`: 你运行HPL后得到的输出文件（仅用于参考校验）

我们将会先module load加载你在make_env.json中指定的所需环境，并使用你提交的`Make.xflops`配置文件在HPL源码文件夹(`source_code/hpl-2.3/`)下运行
```
make arch=xflops
```
并在编译完成后使用你的`HPL.dat`文件替换原有的`HPL.dat`文件，之后在HPL源码文件夹(`source_code/hpl-2.3/`)下运行
```
sbatch hpl.slurm
```
提交你的HPL运行脚本（运行限时1h）得到最终结果。

分数计算方式如下:
|得分点| 满分 | 评分方法
|------|------|-------
| 编译运行 | 50 | 上述评测过程能成功进行并在超时时间(1h)内得到通过正确性验证的HPL运行结果即可得到满分，否则得0分
| 性能 | 50 | 我们将使用HPL运行结果的FLOPS作为性能指标，通过 $\frac{你的性能 - 最低性能}{最高性能 - 最低性能}\times50$ 得到你的得分*

\* 如果只有一人HPL能得到正确的结果，则其性能分按满分计算
\* 由于性能得分计算公式易受极端值影响，只有性能超过[交我算文档](https://docs.hpc.sjtu.edu.cn/app/benchtools/hpl.html#arm)测试性能(865.45GFLOPS)一半，即最终性能成绩超过(432.72GFLOPS)的成绩参与计算最高与最低性能，并进行插值计算分数。如果最终性能低于此值，性能分按照0分计算。

总得分即为编译运行得分与性能得分之和。

> 请注意，HPL.dat提供了测试多组参数的功能，但请确保你提交的HPL.dat中只有一组参数。由于提交了多组参数进行了多次测试从而超时将导致你本题得0分，同时如果多组参数均在限时内完成了测试，我们也只会取第一组参数作为你的性能成绩。

### 评测脚本

在完成了整个流程后如果希望测试你当前的结果，可将所提交文件复制到`source_code/submit`文件夹下，然后运行`python evaluate.py`将会自动完成上述流程并`sbatch`提交任务，如程序未报错（即返回值为0），且对应的提交任务的输出文件`myhpl.out`中成功运行得到了HPL的结果即视为完成编译运行任务。并取`myhpl.out`中的第一个运行成功的HPL的结果为你的性能。