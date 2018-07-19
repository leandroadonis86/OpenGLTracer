# OpenGLTracer
Project using OpenGL to trace glDrawElements() throw injection

Description: This is a source for OpenGL Tracer v1 using count and indeces from
glDrawElements() to locate any 3D object by color it red, great for your game projects.

Compile: Was used on MVS C++ 6.0 since don't need asm code. To use the 2 Method you
may need a diff compiler like Borland Builder C++ or Dev-C++ with asm fuction support.
In some compilers may need to add opengl32 library into your project.

Usage: Use an .EXE injector to aply this .DLL into your aplication or game, 
then use keys as your needs.

Tested: Using...
Windows XP SP2, OpenGL 2.1.8918; Ok
Windows 10, OpenGL 4.1; Ok
Tags: OpenGL, Tracer, Intercept, Hook, Chams.

Key : Function
Numpad 0: Activate eye throw behind the walls;
Numpad 1: Activate filtering objects for count;
Numpad 2: Incrise count filtering;
Numpad 3: Decrise count filtering;
Numpad 4: Activate filtering objects for indices;
Numpad 5: Incrise indices filtering;
Numpad 6: Decrise indices filtering;
Numpad 7: Activate/Desactivate coloring red Object Models;
Numpad 8: Log into a file "log.txt" menu vars;

Copyright (c) July 2018 Leandro Silva <@gmail.com>

Please be kind and support my work for further release, any donation is apreciated
at https://www.paypal.me/LeandroAdonis/5 for 5€ or 5$ amount. Any amount you wish, 
I will be very gratefull and real happy to see my work worth something to you. Thank you.
