
# complete build
all: server cli td kd


# make rule server
server: objs/server.o objs/serverLib.o objs/fileManager.o objs/utils.o objs/validate.o
	gcc -Wall objs/server.o objs/serverLib.o objs/fileManager.o objs/utils.o objs/validate.o -o server

objs/server.o: code/elem/server.c
	gcc -Wall -c code/elem/server.c -Ilib -o objs/server.o

objs/serverLib.o: code/lib/server/serverLib.c
	gcc -Wall -c code/lib/server/serverLib.c -o objs/serverLib.o

objs/fileManager.o: code/lib/server/fileManagement/fileManager.c
	gcc -Wall -c code/lib/server/fileManagement/fileManager.c -o objs/fileManager.o


# make rule client
cli: objs/clientDevice.o objs/clientLib.o objs/utils.o objs/validate.o
	gcc -Wall objs/clientDevice.o objs/clientLib.o objs/utils.o objs/validate.o -o cli

objs/clientDevice.o: code/elem/clientDevice.c
	gcc -Wall -c code/elem/clientDevice.c -Ilib -o objs/clientDevice.o

objs/clientLib.o: code/lib/client/clientLib.c
	gcc -Wall -c code/lib/client/clientLib.c -o objs/clientLib.o


# make rule table
td: objs/tableDevice.o objs/tableLib.o objs/utils.o objs/validate.o
	gcc -Wall objs/tableDevice.o objs/tableLib.o objs/utils.o objs/validate.o -o td

objs/tableDevice.o: code/elem/tableDevice.c
	gcc -Wall -c code/elem/tableDevice.c -Ilib -o objs/tableDevice.o

objs/tableLib.o: code/lib/table/tableLib.c
	gcc -Wall -c code/lib/table/tableLib.c -o objs/tableLib.o


# make rule kitchen
kd: objs/kitchenDevice.o objs/kitchenLib.o objs/utils.o objs/validate.o
	gcc -Wall objs/kitchenDevice.o objs/kitchenLib.o objs/utils.o objs/validate.o -o kd

objs/kitchenDevice.o: code/elem/kitchenDevice.c
	gcc -Wall -c code/elem/kitchenDevice.c -Ilib -o objs/kitchenDevice.o

objs/kitchenLib.o: code/lib/kitchen/kitchenLib.c
	gcc -Wall -c code/lib/kitchen/kitchenLib.c -o objs/kitchenLib.o


# make rule file in comnune
objs/utils.o: code/lib/utils/utils.c
	gcc -Wall -c code/lib/utils/utils.c -o objs/utils.o

objs/validate.o: code/lib/utils/dataValidation/validate.c
	gcc -Wall -c code/lib/utils/dataValidation/validate.c -o objs/validate.o


# pulizia file di compilazione
clean:
	rm objs/*.o server cli td kd