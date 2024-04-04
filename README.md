# NS-3 Docker Image
https://www.nsnam.org/releases/ns-3-29/

Docker recipe for building and starting an ns-3.29 image

## Step 1: Install Docker
First, you need Docker installed on your computer.

**Windows/Mac**: Download Docker Desktop from the official [Docker website](https://docs.docker.com/get-docker/).\
**Linux**: Use your package manager. For ubuntu it would be ```sudo apt-get install docker-ce docker-ce-cli containerd.io```


## Step 2: Build the container
From a terminal/Command prompt, navigate to the folder containing the Dockerfile.

**Mac/Linux**:
```
cd /path/to/the/Dockerfile
```
**Windows**:
```
cd \path\to\the\Dockerfile
```

Then, from inside the same folder as the Dockerfile and compose file, you run the following command to create and build the container:
```
docker compose up --build
```

## Step 3: Accessing the container from a terminal/command prompt
To access the bash terminal of a running Docker container, you can use the ```docker exec``` command.

First find the container ID or name of your running container using the following command:
```
docker ps
```
Thhis command can be run from whatever folder you want.

Then to access the bash shell of the container, run the following:
```
docker exec -it <container_name or container_id> bash
```
NB! Remove the "<>" as well in the command above.

## Step 4: Copying generated files from the container to the host
After you have run the codes from within the container and have generated the .plt file(s), or after running gnuplot and have generated images, run the following command to transfer them to the host machine:
```
docker cp container:src_path dest_path
```
- container: the name or ID of the container you want to copy files from
- src_path: the path on the container of the file you want to copy
- dest_path: the path on your local machine of the directory you want to copy files to

## Another possible way of accessing the container
Another possible way to access the container is to first download Visual Studio Code from the [official website](https://code.visualstudio.com/). Then install the Docker extension from Microsoft. This will allow you to connect to the Docker container from within Visual Studio Code directly, and edit files, download files to your host machine and run terminal commands in the container.

## Additional information

### Viewing additonal information about containers
- For detailed information about a specific container, run the following: ```docker inspect <container_name or Container_id>```\
- To see a live stream of container(s) resource usage statistics, run the following: ```docker stats```\
- To find the size of your containers, including both running and stopped containers, run the following: ```docker ps -as```