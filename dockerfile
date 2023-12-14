# Use the osx-cross/xc-run image
FROM osxcross/xc-run:latest AS builder

# Set the working directory in the container
WORKDIR /usr/src/app

# Copy the source code into the container
COPY . .

# Compile the server and client for macOS
RUN clang -o server_macos server.c
RUN clang -o client_macos client.c

# Use a minimal image for the final stage
FROM alpine:latest

# Set the working directory in the final stage
WORKDIR /usr/src/app

# Copy the compiled binaries from the builder stage
COPY --from=builder /usr/src/app/server_macos ./server_macos
COPY --from=builder /usr/src/app/client_macos ./client_macos

# Expose the port on which the server will run
EXPOSE 8080

# Run the server by default
CMD ["./server_macos"]
