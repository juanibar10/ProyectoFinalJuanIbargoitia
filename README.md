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

One of the main challenges was making the game more than just a basic version, by adding several complete features.

### Blocks with different durability
I implemented blocks that do not all break the same way: some are destroyed with one hit, while others require several impacts. This is shown with different colors to make it clear visually.

### Loading levels from files
Levels are loaded from a text file, which makes it possible to change the block layout without modifying the code.

### External configuration with JSON
I also added a JSON file to easily change game parameters such as the ball speed or the window size.

### Optimization using an Object Pool
For the visual effects when blocks break, I used an object pooling system instead of constantly creating and destroying objects, which makes the game more efficient.

### Game state management
Another important part was correctly handling the full game flow: menu, gameplay, win, game over, and restart without errors.

---

## Feedback and personal opinion

Overall, this project helped me a lot to learn how to structure a complete game in C++.

What I enjoyed the most was adding features like resistant blocks and external level loading because it makes the game more flexible.  
The most difficult part was managing all the game states properly and avoiding bugs when restarting.

I am happy with the result because it is a small but fairly complete project, and it would be easy to expand it with more levels or power-ups.
