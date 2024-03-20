#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  clog << "Your Host is: " << host << ", visit path is : " << path << "\n";
  // find the host address
  Address hostAddress = Address(host, "http");
  clog << "building socket ...\n";
  // try to build a TCP socket
  TCPSocket socket = TCPSocket();
  socket.connect(hostAddress); // connect to host address
  clog << "local_address: " << socket.local_address().to_string() <<
          "\npeer_address: " << socket.peer_address().to_string() << "\n";
  // begin to write
  string str1 = "GET " + path + " HTTP/1.1\r\n";
  string str2 = "Host: " + host + "\r\n";
  string str3 = "Connection: close\r\n";
  string end = "\r\n";
  socket.write(str1);
  socket.write(str2);
  socket.write(str3);
  socket.write(end);
  // begin to read
  string output;
  while (! socket.eof()) {
    socket.read(output);
    cout << output;
  }
  socket.close();
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
