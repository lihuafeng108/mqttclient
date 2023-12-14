
include ../makefile_common.mk

CXX := g++

TARGET = mqtt_client1

OBJDIR := ./obj
SRCDIR := ./ ../so_mqtt_api
# 创建目录,确保目录存在
$(shell mkdir -p ${OBJDIR})

#############################################################
#            工程专用编译和链接标识
#############################################################
CFLAGS_EXTRA  = -fPIC -g -Wall -I../so_mqtt_api \
							   -I../../libmqttclient/include/common \
							   -I../../libmqttclient/include/common/log \
							   -I../../libmqttclient/include/config \
							   -I../../libmqttclient/include/mbedtis \
							   -I../../libmqttclient/include/mbedtis\wrapper \
							   -I../../libmqttclient/include/mqtt \
							   -I../../libmqttclient/include/mqttclient \
							   -I../../libmqttclient/include/network \
							   -I../../libmqttclient/include/platform/linux
LDFLAGS_EXTRA = -ldl -lm -lrt  -lpthread -L../../libmqttclient/lib -lmqttclient

CXXFLAGS += ${CFLAGS_EXTRA}
LDFLAGS += ${LDFLAGS_EXTRA}

#############################################################
#             工程源码目录
#############################################################

#需要过滤的目录 和 目录下的全部 c 文件
FILTERDIR :=
FILTERSOURCES := $(foreach dir, $(FILTERDIR), $(wildcard $(dir)/*.c))
FILTERSOURCES +=

#这里递归遍历5级子目录
DIRS := $(shell find $(SRCDIR) -maxdepth 5 -type d)

#将每个子目录添加到搜索路径
VPATH = $(DIRS)

#查找src_dir下面包含子目录的所有c文件
CPPSOURCES = $(foreach dir, $(VPATH), $(wildcard $(dir)/*.cpp))
CPPOBJECTS = $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(notdir $(filter-out $(FILTERSOURCES),$(CPPSOURCES)))))
DEPENDS = $(CPPOBJECTS:.o=.d)


all: extra_info ${OBJDIR}/${TARGET}
ifneq ($(MAKECMDGOALS),clean)
-include ${DEPENDS}
endif

${OBJDIR}/${TARGET}: ${CPPOBJECTS}
	$(call print_target_bin_info)
	@${CXX} -o $@ ${CPPOBJECTS} ${LDFLAGS}

#编译之前要创建OBJ目录，确保目录存在
$(OBJDIR)/%.o:%.cpp
	$(call print_obj_info)
	@$(CXX) -o $@ -c $< $(CXXFLAGS)

${OBJDIR}/%.d:%.cpp
	@rm -f $@
	@${CXX} -MM $< > $@.temp ${CXXFLAGS}
	@sed 's,\($*\)\.o[:]*,${OBJDIR}/\1.o $@:,g' < $@.temp >$@
	@rm -rf $@.temp

.PHONY : extra_info clean
extra_info:
	$(call print_compiler_exter_info)

clean:
	$(call print_clean_bin_info)
	@rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d ${OBJDIR}/${TARGET}
