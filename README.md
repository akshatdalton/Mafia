# OS-WebServer
A simple multi-threaded webserver to illustrate the use of process scheduling and synchronisation and other OS concepts.

# How to Run?

- Run `make`.
- Run server: `./web_server.c`
- The server will be exposed to the port: `18000`.
  
# Exposed APIs:

- `http://localhost:18000/` to view Reader-Writer.
- `/reader?line_num={line_num}` api for reader to read line number: `line_num`.
- `/writer?line_num={line_num}&content={content}` for writer to edit the line number: `lin_num` by `content`.
- `/docs_writer?line_num={line_num}&content={content}` same as above.
- `/docs_reader` read all the content of the `data.txt` file.

# Instructions to Run
1. Clone the repo.
2. Run the `make` command.
3. If you got no compilation errors then you can run `./web_server`.
4. Go to `localhost:18000/index.html` to view the results.

# Changes to make while editing the codebase
1. Add the new file in `Makefile`, if any.
2. Keep standard libraries in `*.h` header files.
3. Use similar code format throughout the whole codebase (Use VS Code standard format support: `Ctrl + Shift + I`).
4. Use meaningful variable names.
5. Before pushing the code make sure your are on the upstream and avoid doing force push unless necessary.
6. Add executable files (like `web_server`) in `.gitignore`.
