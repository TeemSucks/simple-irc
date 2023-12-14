clang -o client_macos client.c
clang -o server_macos server.c
docker run -v "$(pwd)":/usr/src/app -w /usr/src/app gcc gcc -o client_linux client.c