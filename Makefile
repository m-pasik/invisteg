SHELL = /bin/sh

OBJS = invisteg.o
CFLAGS = -Wall -O3
LDFLAGS = -lpng

EXEC_NAME = invisteg
INSTALL_NAME = invisteg
USER_INSTALL_DIR = ${HOME}/.local/bin
ROOT_INSTALL_DIR = /usr/local/bin

default: ${EXEC_NAME}

${EXEC_NAME}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJS} -o $@

install: invisteg
ifneq ($(shell id -u), 0)
	@echo "Installing ${EXEC_NAME} to ${USER_INSTALL_DIR}/${INSTALL_NAME}."
	@install -m 755 ${EXEC_NAME} ${USER_INSTALL_DIR}/${INSTALL_NAME}
	@echo "Installed ${INSTALL_NAME} locally."
else
	@echo "Installing ${EXEC_NAME} to ${ROOT_INSTALL_DIR}/${INSTALL_NAME}."
	@install -m 755 ${EXEC_NAME} ${ROOT_INSTALL_DIR}/${INSTALL_NAME}
	@echo "Installed ${INSTALL_NAME} globally."
endif

clean:
	rm -f ${OBJS} ${EXEC_NAME}
