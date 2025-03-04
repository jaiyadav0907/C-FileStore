# Object Store

This project implements a simple object store server and client. The server allows clients to upload, download, delete, and list objects in buckets. The client provides a command-line interface to interact with the server.

## Project Structure

```
ObjectStore/
├── client/
│   ├── client.c
│   ├── client.h
│   ├── main.c
│   ├── makefile
│   ├── utils.c
│   └── utils.h
└── server/
    ├── http_server.c
    ├── http_server.h
    ├── logger.c
    ├── logger.h
    ├── main.c
    ├── makefile
    ├── object_store.c
    └── object_store.h
```

## Server

The server is implemented in the `server` directory. It provides the following functionalities:

- **Create Bucket**: Create a new bucket to store objects.
- **Upload Object**: Upload an object to a specified bucket.
- **Download Object**: Download an object from a specified bucket.
- **Delete Object**: Delete an object from a specified bucket.
- **List Objects**: List all objects in a specified bucket.

### Building the Server

To build the server, navigate to the `server` directory and run:

```sh
make
```

This will create an executable named `s3_object_store`.

### Running the Server

To run the server, execute the following command:

```sh
./s3_object_store
```

The server will start listening on port 9000.

## Client

The client is implemented in the `client` directory. It provides a command-line interface to interact with the server.

### Building the Client

To build the client, navigate to the `client` directory and run:

```sh
make
```

This will create an executable named `s3_object_store_client`.

### Running the Client

To run the client, execute the following command:

```sh
./s3_object_store_client
```

The client will prompt you to enter the server IP and port in the format `<ipaddress>:<port>`. After connecting to the server, you can use the following commands:

- **PUT <bucket> <object> <file>**: Upload a local file as an object to a bucket.
- **GET <bucket> <object> <file>**: Download an object from a bucket to a local file.
- **DELETE <bucket> <object>**: Delete an object from a bucket.
- **LIST <bucket>**: List all the objects in the bucket.
- **EXIT**: Exit the application.

### Example Usage

```sh
file-storage> PUT mybucket myobject myfile.txt
file-storage> GET mybucket myobject downloadedfile.txt
file-storage> DELETE mybucket myobject
file-storage> LIST mybucket
file-storage> EXIT
```

## Cleaning Up

To clean up the build files, navigate to the respective directory (`server` or `client`) and run `make clean`:

```sh
make clean
```

This will remove the compiled executables and object files.

