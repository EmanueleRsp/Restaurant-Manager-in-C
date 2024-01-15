# Restaurant Manager: a client-server distributed application in C language

The project is structured as a **client-server application**, where the **multi-service server** manages the incoming requests from three types of devices (client, table and kitchen), communicating by exchanging messages, using specific protocols and data structures. The server is also responsible for managing the data, which are saved in **text files**.

- [Restaurant Manager: a client-server distributed application in C language](#restaurant-manager-a-client-server-distributed-application-in-c-language)
  - [Documentation](#documentation)
  - [Project structure](#project-structure)
  - [How to run the application](#how-to-run-the-application)
  - [Final evaluation and comments](#final-evaluation-and-comments)


## Documentation

> _This project was developed during "Computer Networks" course for the Bachelor's degree in Computer Engineering at the University of Pisa, so inner workings and implementation details are described in **italian**._

The main documentation of the project is available [here](/docs/documentation.md): it contains a quite detailed description of the application, its functionalities and the protocols used for the communication between the devices.

For more details about the **data storaging** by server, please refer to the [relative documentation](/docs/files%20organization.md).

For more details about the data structures used for **data transmission**, please refer to the [relative documentation](/docs/data%20transmission.md).

If you want to check the tasks required for the project, you can find them in the pdf file [here](/docs/Specifiche.pdf).


## Project structure

The project is structured in the following way:
- **`code/`** folder contains the **source code of the application**, which is in turn divided into multiple files:
  - **`code/elem/`** contains the four main programs for each type of device (_client_, _table_, _kitchen_ and _server_);
  - **`code/lib/`** contains the source code of the libraries used by the programs, logically grouped by functionality.
- **`docs/`** folder contains the **documentation** of the project.
- **`files/`** folder contains the **data** used by the application stored by the server (more details in the [relative documentation](docs/files%20organization.md)).
- **`objs/`** is the folder in which the **object files** are stored after the compilation of the source code.
- **`exec.sh`** is the **script** used to compile and run the application.
- **`Makefile`** is the **makefile** used to compile the source code or **delete object/exe files**.
- **`README.md`** is the **file** you are reading right now :).


## How to run the application

You can download the latest zip file released from [here](https://github.com/EmanueleRsp/Restaurant-Manager-in-C/releases). 

Make sure to have installed the `gcc` compiler to compile C files:
```bash
gcc --version
```
Else, you can install it with the following command:
```bash
sudo apt-get update
sudo apt-get install gcc
```

To compile and run the application automatically, you can use the **script** `exec.sh`:
```bash
./exec.sh
```

To possibly **delete** object and exe files, you can use:
```bash
make clean
```


## Final evaluation and comments

The project was evaluated with a **score of 5/5 with honours**.

The professor specifically appreciated the **modularity** of the application and the **attention to memory management**, expecially the **fragmentation** approach used to store data in files. He also appreciated the **documentation** of the project, which is quite detailed, well-structured and also faces up some critical points of the application. 
