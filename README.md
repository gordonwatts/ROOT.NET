ROOT.NET

Introduction
============

ROOT is a general utility for data analysis and contains many features that help with high-speed data processing,
signal/background separation, and, of course, plotting. ROOT is written in C++ and comes with a complete meta-data system.
This utility makes use of that meta-data system to generate a fully typed interface to give access to most of ROOT in the .NET world.
See the documentation page for some simple examples.

All ROOT.NET packages are released on the NuGet web server. To download and access, open up the Visual Studio 2010 NuGet pacakge
dialog in a .NET package, and enter the ROOT version number ("v5.34.34", for example) into the search box. Install what you need!

Usage
=====

Everything exists under the ROOTNET namespace. All objects and namespaces have a "N" in front of them (e.g. NTTree, for example).

   - TCollection and its ilk expose iterators for use in loops and LINQ.
   - A TTree exposes an interator. Use the C# "dynamic" keyword and you can access simple leaves. It is horribly slow, but for quick tests and the like it works well.
   - GetXXX methods are also replicated as a property called XXX. If there is also a SetXXX which takes a single argument, then you can also assign a value to the property.
   - Index operator[] is carreid over, as is an attempt at operator overloading (I'm afraid that isn't complete due to semantic limitations in .NET).
   - To get around the (blissful) lack of multiple inherritance in .NET's object model, liberal use of Interfaces is made.
   - Namespaces in ROOT are supported (e.g. TMVA), but template classes are not.
   - Pointers returned from ROOT are tracked, and reference counted (well, .NET does the reference counting). This can lead to funny behavior if you create a historam and then gc it when it is attached to a file: it will get deleted before you've had a chance to call TFile::Write.
   - Global variables (gSystem, etc,) are also written out. Some ROOT globals are macros, so those aren't written out.


History
=======
This project was copied from codeplex, where it started (in svn, then in hg):

	https://rootdotnet.codeplex.com/
	
Only the head was copied over, so all the history beyond the head remains on codeplex.
