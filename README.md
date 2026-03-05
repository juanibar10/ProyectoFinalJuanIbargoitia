# Report — Final Project Arkanoid (C++ / SFML)

---

## Brief description

For my final project, I developed an **Arkanoid / Breakout** style game in **C++** using the **SFML** library.

The player controls a paddle and must bounce a ball to break all the blocks in the level. The game includes a lives system, scoring, and different types of blocks depending on their durability, represented with colors.

---

## Controls

- **Left / Right arrow keys** → move the paddle  
- **ENTER** → start or restart the game  
- **ESC** → return to the menu when the game ends  

---

## Main technical challenges

One of the main challenges was making the game more than just a basic version, by adding several complete features and making sure they worked correctly together.

### Blocks with different durability
I implemented blocks that do not all break the same way: some are destroyed with one hit, while others require several impacts. This is shown with different colors to make it clear visually.

### Loading levels from files
Levels are loaded from a text file, which makes it possible to change the block layout without modifying the code.

### External configuration with a simple text file
Game parameters (window size, ball speed, colors, etc.) are loaded from a simple **key=value** config file:
- `assets/config/game.txt`

This keeps the configuration external and easy to modify without requiring extra libraries.

### Break effect and power-ups
When a brick is destroyed, a short break effect is spawned. Bricks can also drop a power-up (MultiBall), which spawns extra balls when collected.

### Game state management
Another important part was correctly handling the full game flow: menu, gameplay, win, game over, and restart.

Special attention was paid to resetting the game state properly, recovering from game over or level completion, and allowing the player to restart the game without errors or inconsistent behavior.

---

## Feedback and personal opinion

Overall, this project helped me a lot to learn how to structure a complete game in C++.

What I enjoyed the most was adding features like resistant blocks, external level loading, and configuration files because they make the game more flexible.

The most difficult part was managing collisions and game states correctly, especially making sure that the game recovers properly when restarting or changing levels without introducing bugs.

I am happy with the result because it is a small but fairly complete project, and it would be easy to expand it with more levels or additional power-ups in the future.