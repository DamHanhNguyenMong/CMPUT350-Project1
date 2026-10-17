# CMPUT 350 HW 2
### AI Usage 
AI was used to assist with documentation, including brief code comments. No AI-generated code or core project functionality was used.
AI was also used to generate the initial draft of this readme. 

Referenced this website: https://www.aleksandrhovhannisyan.com/blog/implementing-a-finite-state-machine-in-cpp/

### OVERVIEW
This is a Galaga-style arcade shooter built on the course game engine (SFML). The whole game is started by adding a single Galaga object to the engine. Galaga creates and removes every other game object itself: the stars, the player, the enemies and (through the player and enemies) the bullets.

How to play
  5      insert a coin
  1      start the game (needs a coin)
  A / D  move the ship left / right (hold to keep moving)
  Space  shoot (at most two player bullets on screen at once)

You start with 3 lives. Clear every enemy to move on to the next level. Enemies dive at you and shoot; you lose a life when you are hit by an enemy bullet or fly into an enemy.

How to build and run
  cmake --build build
  ./build/Project1

### GALAGA STATE FLOW
Galaga owns a finite state machine. Each state is its own class (GalagaState subclass) with enter(), update(), draw(), onNotification() and exit().

States:
- WAITING: Title screen. Pressing 5 adds a credit, pressing 1 with a credit starts a new game. (MOVES TO STARTUP)
- STARTUP: Shows the level number and score, and spawns the player ship if there isn't one. After 2 seconds it moves on. (MOVES TO WAVES)
- WAVES: Spawns 5 waves of 8 enemies. Each enemy flies in along its own Bezier curve to its slot in the 4x10 formation. If the player dies during the waves, enemies keep entering and the death is handled when GAMEPLAY begins. Once everything is spawned and every enemy has reached its slot, we move to GAMEPLAY. (MOVES TO GAMEPLAY)
- GAMEPLAY: Every 4 seconds two enemies are chosen to leave the formation and attack. When all enemies are dead the level goes up and we return to STARTUP. If the player dies we go to RESPAWN, or to GAME OVER if there are no lives left. (GOES TO 1 OF STARTUP, RESPAWN, GAMEOVER)
- RESPAWN: Waits until every attacking enemy is back in formation, spawns a new player, and shows how many lives are left for 2 seconds. (GOES TO GAMEPLAY)
- GAME OVER: Shows the final score for 4 seconds, removes everything from the field (RemoveAll notification) and returns to WAITING. (GOES TO WAITING)

### DESIGN DECISIONS AND CHANGES TO THE BASIC API
Galaga and the state machine
 - Galaga is a GraphicsObject so the engine updates and renders it; it draws the current state's text in RenderForeground.
 - The current state is a std::unique_ptr<GalagaState>. States hold only the data they need (for example the Waves state keeps its wave counter and next spawn frame) - game-wide data (score, lives, level, credits, enemies alive) lives in Galaga.
 - Transitions are deferred: setState() only records the request and the change is applied at the end of Update or ReceiveNotification. This is so that a state is never destroyed while one of its own functions is still running.
 - All timing is in frames using the engine's shared frame counter (30 fps).
 - Galaga installs every keyboard event (5, 1, A, D, Space) once, in its Initialize. Players only register for them.

Notifications
- Enemies tell Galaga about their death with one notification per enemy type and place of death - e.g. Enemy1DiedAttacking. Galaga sets the points for each, so no number is ever sent through the notification manager.
- More points are awarded for an enemy killed while out of formation:
    - Enemy1 : 50 in formation / 100 attacking
    - Enemy2 : 80 / 160
    - Enemy3 : 150 / 300
- Player sends PlayerDied, Galaga sends RemoveAll to clear the field, and the player and enemies remove themselves.

Player
 - Bounds and collision shapes are in local coordinates around (0,0) - the engine moves them using GetLocation(). The ship is drawn around (0,0) as well.
 - Key repeat is disabled in the engine, so a held key sends only one notification. We added Stop notifications (StopLeft/StopRight on key release): the move notifications set a movement and Update moves the ship every frame.
 - The player is kept inside the window bounds.

Enemies
- Enemy is an abstract base class with a virtual bool Attack(GameContext*, const Point2D& target) which returns false if the enemy can't start an attack (dead, or not information). The constructor is Enemy(loc, path): loc is the formation slot and path is the Bezier curve it follows to get there.
- Everything common lives in the base class: following Bezier curves (location and rotation), the formation sway, health, collisions and the death notification. Enemy1/2/3 only supply their attack path, firing pattern and drawing.
- Constant speed: a raw Bezier parameter doesn't move at constant speed, so each path is sampled into an arc-length table and every frame the enemy advances afixed number of pixels along it. Rotation comes from the curve's slope.
- Modes: entering -> inFormation -> attacking -> returning -> inFormation. After a dive an enemy leaves the bottom of the screen, reappears above the top and flies back to its slot.
- In formation the enemies hover left and right. All of them compute the offset from the same frame counter, so the whole formation sways together.
- The three types:
  - Enemy1 (1 health): zig-zag dive - one bullet straight down every 1.5 s
  - Enemy2 (1 health): swoop toward the player - aimed shot every 40 frames
  - Enemy3 (2 health): wide sweep then a dive - 3 bullet spread every 2 s. Blue at full health, red after one hit.
    
- Enemies are placed by formation row (type 3 on top, type 2 in the middle, type 1 at the bottom), so all three types are on screen together.
- Enemies only collide with the player (they are destroyed) and with player bullets (1 damage). Enemy bullets pass through other enemies and they don't collide with one another.

Bullets
 - Constructor: Bullet(location, heading, isPlayerBullet, parent). The parent is the object that fired it, the bullet is destroyed when its parent is destroyed.
 - Collision handling in Enemy/Player doesn't kill the bullet; the bullet removes
   itself in LateUpdate so it can see all of that frame's collisions first.
 - Player bullets are yellow and enemy bullets are orange.

DrawContext and the engine
 - Every public draw call takes local coordinates and is transformed (rotate, then translate) by the DrawContext before drawing.
 - The engine sets context.CurrObject to the object being initialised before calling Initialize, so objects can register themselves with the notification manager.

Bezier
- Each enemy builds curve from its control points (3n+1 points for n segments, t from 0 to n).
- t doesn’t give constant speed, so each path is sampled into a table. Each frame the enemy moves a fixed number of pixels, and the distance is converted back to t.
- Position comes from GetPoint(t). Rotation comes from atan2 of GetSlope(t) plus π/2.
- Each enemy follows separate curves for entering formation, attacking and returning, and each enemy type has its own attack path.

### GROUP MEMBER CONTRIBUTIONS
- Zoe: Galaga state machine, Player, DrawContext, Enemy
- Hanh: Bezier, Bullets, Notification manager, Engine
