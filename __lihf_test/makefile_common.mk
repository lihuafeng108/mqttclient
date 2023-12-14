#description : 各模块编译过程输出信息函数定义
#author :  lihf
#date : 2022.08.19

SHELL=/bin/bash

INDENT_LAYER1 = "  "
INDENT_LAYER2 = "          "
CUR_PRJ_DIR_NAME  := $(notdir $(shell pwd))

define print_prj_info
	@echo " "
	@echo -e "${CYAN_HEAVY}"${INDENT_LAYER1}"  |- ${CUR_PRJ_DIR_NAME} ${NO_COLOR}"
endef

define print_target_lib_info
	@echo -e "${PURPLE_HEAVY}"${INDENT_LAYER2}"linking... " ${ROBOT_LIB_PATH}/${TARGET} "${NO_COLOR}"
endef

define print_target_bin_info
	@echo -e "${PURPLE_HEAVY}"${INDENT_LAYER2}"linking... " ${OBJDIR}/${TARGET} "${NO_COLOR}"
endef

define print_obj_info
	@echo -e "${GREEN}"${INDENT_LAYER2}"compiling    " $< "${NO_COLOR}"
endef

define print_compiler_exter_info
	@echo -e "${CYAN}"${INDENT_LAYER2}"CFLAGS_EXTRA:${CFLAGS_EXTRA}${NO_COLOR}"
	@echo -e "${CYAN}"${INDENT_LAYER2}"LDFLAGS_EXTRA:${LDFLAGS_EXTRA}${NO_COLOR}"
endef

define print_clean_lib_info
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm $(OBJDIR)/*.o"
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm $(OBJDIR)/*.d"
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm ${ROBOT_LIB_PATH}/${TARGET}${NO_COLOR}"
endef

define print_clean_bin_info
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm $(OBJDIR)/*.o"
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm $(OBJDIR)/*.d"
	@echo -e "${GREEN}"${INDENT_LAYER2}"rm ${OBJDIR}/${TARGET}${NO_COLOR}"
endef

define print_finish_info
	@echo -e "${CYAN_HEAVY}"${INDENT_LAYER1}"     ${CUR_PRJ_DIR_NAME} build success...${NO_COLOR}"
endef
