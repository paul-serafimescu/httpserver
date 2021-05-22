# httpserver

simple HTTP server for UNIX

## Dependencies

* [sqlite](https://sqlite.org/index.html)
* [json-c](http://json-c.github.io/json-c/json-c-current-release/doc/html/index.html)
* [Node.js](https://nodejs.org/en/) (optional example frontend)
* [yarn](https://yarnpkg.com/) (optional example frontend)

## Building

`make` will build the server executable

## Running

`make run` will build and run the executable

## Example Frontend

`wwwroot/example-frontend` contains a React frontend. Build this by first installing dependencies with `yarn install` within the directory, followed by `yarn run build`. This generates the client-side JavaScript. `examples/example_data.sql` contains starting data for the frontend which can be read directly into the SQLite3 database.

## Cleaning Up

`make clean`
