NATIVE_JAVA_CLASSES = com.alipay.kepler.fasterkv.FasterKv


OUTPUT = target
MAIN_CLASSES = $(OUTPUT)/classes
MAIN_SRC = src/main/java
FASTER_JNI_INCLUDE = ./include
NATIVE_INCLUDE = -I ../cc/src
NATIVE_SRC = ./fasterjni

# todo
JNI_INCLUDE = -I /usr/lib/jvm/default-runtime/include/ -I /usr/lib/jvm/default-runtime/include/linux/

all:
	mkdir -p $(MAIN_CLASSES)
	javac -d $(MAIN_CLASSES)\
		$(MAIN_SRC)/com/alipay/kepler/fasterkv/*.java
	javah -cp $(MAIN_CLASSES) -d $(FASTER_JNI_INCLUDE) -jni $(NATIVE_JAVA_CLASSES)
	g++ $(NATIVE_INCLUDE) -I ./ $(JNI_INCLUDE) -l faster -l tbb -l uuid -L ../cc -fPIC -shared -o libfasterkvjni.so $(NATIVE_SRC)/*

clean:
	rm -rf include/*
	rm -rf $(OUTPUT)
	rm -rf libfasterkvjni.so
