#uapar
##Intro
uapar([UESTC](http://www.uestc.edu.cn/)-APAR) is a large scale IP alias resolution tool, which is based on  [APAR(Analytic and Probe-based Alias Resolver)](http://itom.utdallas.edu/papers/ToN_APAR.pdf) and make a few improvements

As we know, a router has more than one interface, and each interface has an IP address. So what alias resolution do is to dentify IP addresses belonging to the same router

##Features
- writen in C
- use hash table and dymatic array to optimize storage and search
- use more strict rules to resolute IP alias pairs

##Build
uapar can only be complied and used in Linux and unix-like OS

it is as simple as 

	make

##Usage
there are only three options for uapar
- -l: followed by a number(1 to 5) which stands for 5 log level, more info about uapar's log see `up_log.h`. By default, log level is `LOG_WARNING`
	- 1 for `LOG_TRACE`	
	- 2 for  `LOG_DEBUG`
	- 3 for	`LOG_WARNING`
	- 4 for `LOG_ERROR`
	- 5 for `LOG_STATE`	
- o: followed by a filename which stands for the log file, by default,  log will be output through `stderr`
- i: followed by a set of string which stands for the input file
###Note 
input file's format should like this:

```
  218.241.107.98	182.125.83.140	218.241.107.97	218.241.98.45	192.168.1.253	159.226.253.73	159.226.253.42	219.158.34.53	219.158.11.121	219.158.96.18	61.168.194.50	222.142.62.130	
218.241.107.98	113.3.242.221	q	218.241.107.97	218.241.98.45	192.168.1.253	159.226.253.77	159.226.253.46	219.158.34.53	219.158.11.93	q	q	219.158.21.106	60.14.244.82	221.206.0.122	
```
- each line stands for an IP path
- annoymous ip should be represent as `q`

###Tips
if you use the data produced by [scamper](http://www.caida.org/tools/measurement/scamper/), you can use my awk script `scamper_filter.awk`(under `util` folder) to get ip path from output data of scamper.
