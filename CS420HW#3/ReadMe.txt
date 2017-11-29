Assignment #3: Ray tracing
Use 1 late day

!!!Operating System: Max OS X !!!!!

FULL NAME: Yuzhou Ge
ID: 7057669325


MANDATORY FEATURES
------------------

<Under "Status" please indicate whether it has been implemented and is
functioning correctly.  If not, please explain the current status.>

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  Yes Functioning Correctly

2) Ray tracing sphere                     Yes Functioning Correctly

3) Triangle Phong Shading                 Yes Functioning Correctly

4) Sphere Phong Shading                   Yes Functioning Correctly

5) Shadows rays                           Yes Functioning Correctly

6) Still images                           Yes Functioning Correctly(Inside StillImage Folder)
   
7) Extra Credit (up to 20 points)
	
   Recursive reflection
   I did the recursive reflection in function "traceRay". In order to use this function: when running the program in command line, add one more argument"-OR" in the end. !!!!!!!!!!!
   E.g: "./assign3 test.scene.txt test.jpg -OR"

   Good antialiasing
   I also have a good antialiasing by using the technqiue of super sampling. 
   You can check from the still images, the boundaries are smooth.

Still Images:
001: from screenfile.txt, no antialiasing applied, no recursive reflection
002: from screenfile.txt, with antialiasing applied, no recursive reflection
003: from test2.scene.txt, with antialiasing applied, no recursive reflection
004: from test2.scene.txt, with antialiasing applied, with recursive reflection
005: from spheres.scene.txt, with antialiasing applied, no recursive reflection
006: from spheres.scene.txt, with antialiasing applied, with recursive reflection
007: from table.scene.txt, with antialiasing applied, no recursive reflection
008: from SIGGRAPH.scene.txt, with antialiasing applied, no recursive reflection