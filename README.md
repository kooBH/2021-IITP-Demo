# 2021-IITP-Demo  

## Clone & Build 

```
# get - note : algorithm submodules require access to private repositories  
git clone https://github.com/kooBH/2021-IITP-Demo
cd 2021-IITP-Demo
git submodule init
git submodule update

# build
cd bin
cmake ..
```

## ASR key    
   
requires ASR API key from https://aiopen.etri.re.kr/  

+ private/config.json  
``` json
{
	"key1"  :"your-key-1"
	,"key2" :"your-key-2"
	,"key3" :"your-key-3"
	,"key4" :"your-key-4"
}
```