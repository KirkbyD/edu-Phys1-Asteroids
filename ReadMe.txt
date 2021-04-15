Dylan Kirkby

INFO6019 – Physics 1 
Midterm Exam – Monday, October 28th, 2019

Questions answered in project AmethystEngine.

AmethystEngine by:
	Dylan Kirkby,
	Brian Cowan,
	Ivan Parkhomenko

Created in Visual Studio Community 2019
Runs in x64 only, Release recommended for performance.


Camera default spawn set to the requested position, but I opted to keep it controllable.
It tends to jump a bit on initial load - recommend not moving cursor while it is loading.
I've also noticed rare 'teleporting' on load and am unsure of the cause. 
This can be fixed by following the asteroids to the space station if visible, or relaunching.


Movement controls are relative to the camera's current angle. 
	Mouse	Adjust viewing angle
	W	Move Forwards
	S	Move Backwards
	A	Move Left
	D	Move Right
	Q	Move Up
	E	Move Down
	F	Engage Laser (Fire Everything)



ALL QUESTIONS IN ONE SOLUTION
All non trivial operations in Physics/cPhysics.cpp
This documents gives functions names and lines to where the questions are answered.
No changes to code necessary, though changing line 9, 'this->AsteroidSpawnRate = 1.0f;' 
	to say 0.2f may be beneficial for demonstration purposes at the cost of some performance.



Q1:
Rendered using Parts_Exposed_to_Asteroids and Parts_Safe_from_Asteroids
specific configurations in AmethystEngine\AmethystEngine\data\config\models\models_physics_midterm.ini
to be sent to thge cFileManager's BuildObjects() function.

Static asteroids rendered to top right of the space station. (In initial orientation)

Parts_Exposed_to_Asteroids registered with physicsShapeType as MESH for use with collision in the following questions.
The other 4 models (safe parts and asteroids 11/14/15) are Typed UNKNOWN, as they are only for display purposes.



Q2: 
Handled in Physics/cPhysics.cpp starting at line 60 in IntegrationStep().
objects are queued for deletion in TestForCollisions() at line 403.

As the station is static, numbers are gathered for its dimensions in the cPhysics constructor, lines 18 - 42



Q3:
Animation handled in IntegrationStep starting at line 102.

Initially tried to do MESH - MESH collision but failed, and hard coded some values per asteroid tied to scale for a SPHERE - MESH collision.

Tests found in:
TestForCollisions line 384.
DoSphereMeshCollisionTest 456 - 478.
Calls MidtermExplosion() ln 543, which creates the explosion object via factory call and sets a few things.



Q4:
Bound to F key.
Laser is on a timer for refiring and being deleted. Set a bit longer than I'd like due to slowdowns from the bonus.
Animations in IntegrationStep, 113 - 125

Checks in TestForCollisions() 367 - 382.

Q4 is coupled with Q5:
MidtermFutureHitScan() checks if thze asteroid will collide before disabling the model's collision detection or shooting it with the laser.
Found on Line 587.
Theres some nested ifs and for loops in an attempt to optimize the number os scans needed based on the asteroid size.
An Asteroid 14 mesh passing through the station ring is still quite the slowdown unfortunately.
If collision is found the function returns true immediately after calling MidtermLaserBlast()

MidtermLaserBlast() ln 552 Creates an explosion similar to MidtermExplosion() with a different colour, focused on the asteroid.
It also sets a velocity/inverse mass while disabling collision because the explosion just stopping looked somewhat odd!
Then it finds the direction to draw the laser, and loops drawing small blue spheres along the
line till within an acceptable distance of the target asteroid.
Finally it turns the asteroid visibility off, which queues it for deletion and disables collision.





