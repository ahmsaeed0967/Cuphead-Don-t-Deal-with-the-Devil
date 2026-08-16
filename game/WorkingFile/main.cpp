#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;
using namespace sf;

// Custom coordinates replacement
struct Coordinates {
    float x, y;

    Coordinates() : x(0), y(0) {}
    Coordinates(float x_, float y_) : x(x_), y(y_) {}

    Coordinates operator+(const Coordinates& other) const {
        return Coordinates(x + other.x, y + other.y);
    }

    Coordinates operator-(const Coordinates& other) const {
        return Coordinates(x - other.x, y - other.y);
    }

    Coordinates operator*(float scalar) const {
        return Coordinates(x * scalar, y * scalar);
    }

    Coordinates operator/(float scalar) const {
        return Coordinates(x / scalar, y / scalar);
    }
};

// Custom Rectangle replacement
struct Rectangle {
    float left, top, width, height;

    Rectangle() : left(0), top(0), width(0), height(0) {}
    Rectangle(float l, float t, float w, float h) : left(l), top(t), width(w), height(h) {}

    bool contains(float x, float y) const {
        return (x >= left && x <= left + width && y >= top && y <= top + height);
    }

    bool intersects(const Rectangle& other) const {
        return !(left + width < other.left ||
            other.left + other.width < left ||
            top + height < other.top ||
            other.top + other.height < top);
    }
};


// Custom Color replacement
struct MyColor {
    unsigned char r, g, b, a;

    MyColor() : r(0), g(0), b(0), a(255) {}
    MyColor(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {
    }

    // Common colors
    static MyColor Red() { return MyColor(255, 0, 0); }
    static MyColor Green() { return MyColor(0, 255, 0); }
    static MyColor Blue() { return MyColor(0, 0, 255); }
    static MyColor White() { return MyColor(255, 255, 255); }
    static MyColor Black() { return MyColor(0, 0, 0); }
    static MyColor Yellow() { return MyColor(255, 255, 0); }
    static MyColor Cyan() { return MyColor(0, 255, 255); }
    static MyColor Magenta() { return MyColor(255, 0, 255); }
    static MyColor Transparent() { return MyColor(0, 0, 0, 0); }

    MyColor operator*(float scalar) const {
        return MyColor(static_cast<unsigned char>(r * scalar),
            static_cast<unsigned char>(g * scalar),
            static_cast<unsigned char>(b * scalar),
            a);
    }
};

// ========== Game Constants ==========
struct GameConstants {
    static const int WINDOW_WIDTH = 900;
    static const int WINDOW_HEIGHT = 650;

    // Physics constants
    static const float GRAVITY;
    static const float TERMINAL_VELOCITY;
    static const float JUMP_STRENGTH;
    static const float PLAYER_SPEED;

    // Animation constants
    static const int RUN_FRAMES = 14;
    static const int JUMP_FRAMES = 8;
    static const int IDLE_FRAMES = 1;
    static const float FRAME_TIME;
};

// In your GameConstants initialization:
const float GameConstants::GRAVITY = 0.15f;
const float GameConstants::TERMINAL_VELOCITY = 1.0f;
const float GameConstants::JUMP_STRENGTH = -15.0f;
const float GameConstants::PLAYER_SPEED = 1.0f;
const float GameConstants::FRAME_TIME = 0.075f;

// Structure for leaderboard entries
struct LeaderboardEntry {
    string name;
    int score;
};


const int MAX_ENEMIES = 20;
struct EnemyArray {
    EnemyArray() : size(0) {}

    void add(void* enemy) {
        if (size < MAX_ENEMIES) {
            data[size] = enemy;
            size++;
        }
    }

    void remove(int index) {
        if (index < 0 || index >= size) return;
        for (int i = index; i < size - 1; i++) {
            data[i] = data[i + 1];
        }
        size--;
    }

    void* get(int index) const {
        if (index < 0 || index >= size) return nullptr;
        return data[index];
    }

    int getSize() const { return size; }

    void clear() { size = 0; }

private:
    void* data[MAX_ENEMIES];
    int size;
};

class Leaderboard {
private:
    LeaderboardEntry leaderboard[10];
    int leaderboardSize;

public:
    Leaderboard() : leaderboardSize(0) {}

    void readLeaderboard() {
        ifstream file("leaderboard.txt");
        leaderboardSize = 0;

        if (file.is_open()) {
            string line;
            while (getline(file, line) && leaderboardSize < 10) {
                size_t commaPos = line.find(',');
                if (commaPos != string::npos) {
                    leaderboard[leaderboardSize].name = line.substr(0, commaPos);

                    string scoreStr = line.substr(commaPos + 1);
                    int score = 0;
                    for (char c : scoreStr) {
                        if (c >= '0' && c <= '9') {
                            score = score * 10 + (c - '0');
                        }
                    }
                    leaderboard[leaderboardSize].score = score;
                    leaderboardSize++;
                }
            }
            file.close();
        }

        // Bubble sort for leaderboard
        for (int i = 0; i < leaderboardSize - 1; i++) {
            for (int j = 0; j < leaderboardSize - i - 1; j++) {
                if (leaderboard[j].score < leaderboard[j + 1].score) {
                    LeaderboardEntry temp = leaderboard[j];
                    leaderboard[j] = leaderboard[j + 1];
                    leaderboard[j + 1] = temp;
                }
            }
        }
    }

    void writeLeaderboard() {
        ofstream file("leaderboard.txt");

        if (file.is_open()) {
            for (int i = 0; i < leaderboardSize; i++) {
                file << leaderboard[i].name << "," << leaderboard[i].score << endl;
            }
            file.close();
        }
    }

    void addToLeaderboard(const string& playerName, int score) {
        readLeaderboard();

        LeaderboardEntry newEntry;
        newEntry.name = playerName;
        newEntry.score = score;

        int insertPos = leaderboardSize;
        for (int i = 0; i < leaderboardSize; i++) {
            if (score > leaderboard[i].score) {
                insertPos = i;
                break;
            }
        }

        if (leaderboardSize < 10) leaderboardSize++;

        for (int i = leaderboardSize - 1; i > insertPos; i--) {
            leaderboard[i] = leaderboard[i - 1];
        }

        if (insertPos < 10) {
            leaderboard[insertPos] = newEntry;
        }

        if (leaderboardSize > 10) {
            leaderboardSize = 10;
        }

        writeLeaderboard();
    }

    int getSize() const { return leaderboardSize; }
    const LeaderboardEntry& getEntry(int index) const { return leaderboard[index]; }
};

// Helper function to convert integer to string
string intToString(int num) {
    stringstream ss;
    ss << num;
    return ss.str();
}

class GameObject {
protected:
    Texture texture;
    Sprite sprite;
    bool textureLoaded;

public:
    GameObject(const string& path = "", float x_position = 0.f, float y_position = 0.f,
        float x_scale = 1.f, float y_scale = 1.f) : textureLoaded(false) {
        if (!path.empty()) {
            if (!texture.loadFromFile(path)) {
                cerr << "Failed to load texture: " << path << endl;
                return;
            }
            textureLoaded = true;
            sprite.setTexture(texture);
            sprite.setPosition(x_position, y_position);
            sprite.setScale(x_scale, y_scale);
        }
    }

    void setOriginCenter() {
        if (textureLoaded) {


            float boundsWidth = sprite.getLocalBounds().width;
            float boundsHeight = sprite.getLocalBounds().height;
            sprite.setOrigin(boundsWidth / 2, boundsHeight / 2);
        }
    }

    bool loadTexture(const string& path) {
        if (texture.loadFromFile(path)) {
            textureLoaded = true;
            sprite.setTexture(texture);
            setOriginCenter();
            return true;
        }
        cerr << "Failed to load texture: " << path << endl;
        return false;
    }

    bool isClicked(Event& event, RenderWindow& window) {
        if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
            int mouseX = Mouse::getPosition(window).x;
            int mouseY = Mouse::getPosition(window).y;

            // Use your custom Rectangle struct
            Rectangle bounds = getGlobalBounds();

            // Check if mouse is within bounds
            return (mouseX >= bounds.left && mouseX <= bounds.left + bounds.width &&
                mouseY >= bounds.top && mouseY <= bounds.top + bounds.height);
        }
        return false;
    }

    bool isHovered(RenderWindow& window) {
        Coordinates mousePos({ Mouse::getPosition(window).x , Mouse::getPosition(window).y });

        float bounds_x = sprite.getGlobalBounds().left;
        float bounds_y = sprite.getGlobalBounds().top;
        float bounds_width = sprite.getGlobalBounds().width;
        float bounds_height = sprite.getGlobalBounds().height;


        Rectangle bounds({ bounds_x,bounds_y,bounds_width,bounds_height });
        return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    void setScale(float x_scale, float y_scale) {
        sprite.setScale(x_scale, y_scale);
    }

    void setOrigin(float x, float y) {
        sprite.setOrigin(x, y);
    }

    void move(float offsetX, float offsetY) {
        sprite.move(offsetX, offsetY);
    }

    Coordinates getPosition() const {
        float xVal = sprite.getPosition().x;
        float yVal = sprite.getPosition().y;
        return Coordinates(xVal, yVal);
    }

    Rectangle getGlobalBounds() const {
        float leftVal = sprite.getGlobalBounds().left;
        float topVal = sprite.getGlobalBounds().top;
        float widthVal = sprite.getGlobalBounds().width;
        float heightVal = sprite.getGlobalBounds().height;
        return Rectangle(leftVal, topVal, widthVal, heightVal);
    }

    void draw(RenderWindow& window, bool debug = false) {
        if (textureLoaded) {
            window.draw(sprite);

            // Draw bounding box for debugging
            if (debug) {
                RectangleShape boundsBox;
                boundsBox.setPosition(getGlobalBounds().left, getGlobalBounds().top);
                boundsBox.setSize({ getGlobalBounds().width, getGlobalBounds().height });
                boundsBox.setFillColor(Color::Transparent);
                boundsBox.setOutlineColor(Color::Red);
                boundsBox.setOutlineThickness(2.0f);
                window.draw(boundsBox);
            }
        }
        else {
            // Draw placeholder rectangle if no texture
            RectangleShape rect({ 50, 100 });
            Coordinates pos = getPosition();
            rect.setPosition(pos.x - 25, pos.y - 50);
            rect.setFillColor(Color::Blue);
            window.draw(rect);
        }
    }

    Sprite& getSprite() {
        return sprite;
    }

    bool isTextureLoaded() const {
        return textureLoaded;
    }

    virtual void update(float deltaTime) {
        // Can be overridden by derived classes
    }
};

class Background : public GameObject {
public:
    Background(const string& path = "", float x_position = 0.f, float y_position = 0.f,
        float x_scale = 1.f, float y_scale = 1.f)
        : GameObject(path, x_position, y_position, x_scale, y_scale) {
    }
};

class Button {
    RectangleShape shape;
public:
    Button(float x_position = 0.f, float y_position = 0.f,
        float width = 1.f, float height = 1.f) {
        shape.setPosition(x_position, y_position);
        shape.setSize({ width, height });
        shape.setFillColor(Color::Transparent);
    }

    void draw(RenderWindow& window) {
        window.draw(shape);
    }

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }

    bool isClicked(Event& event, RenderWindow& window) {
        if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
            Coordinates mousePos({ Mouse::getPosition(window).x, Mouse::getPosition(window).y });

            float bounds_left = shape.getGlobalBounds().left;
            float bounds_top = shape.getGlobalBounds().top;
            float bounds_width = shape.getGlobalBounds().width;
            float bounds_height = shape.getGlobalBounds().height;

            Rectangle bounds({ bounds_left, bounds_top, bounds_width, bounds_height });
            return bounds.contains(mousePos.x, mousePos.y);
        }
        return false;
    }

};

class Player : public GameObject {
    struct Information {
        string name;
        int health;
        int coins;
        int score;
    } info;

    // Physics variables
    float velocityY = 0.0f;
    bool onGround = false;
    bool isJumping = false;
    bool facingRight = true;
    float animationTimer = 0.0f;
    int currentFrame = 0;

    // Animation variables
    string currentState = "idle";

    // Animation textures - using arrays
    Texture idleTextures[10];  // Max 10 idle frames
    Texture runTextures[20];   // Max 20 run frames  
    Texture jumpTextures[20];  // Max 20 jump frames

    int idleFrameCount = 0;
    int runFrameCount = 0;
    int jumpFrameCount = 0;

public:
    Player(const string& idlePath = "", float x_position = 0.f, float y_position = 0.f,
        float x_scale = 1.f, float y_scale = 1.f)
        : GameObject(idlePath, x_position, y_position, x_scale, y_scale) {
        info.name = "Player";
        info.health = 3;
        info.coins = 0;
        info.score = 0;

        // Reset counts
        idleFrameCount = 0;
        runFrameCount = 0;
        jumpFrameCount = 0;

        // Initialize animations
        loadAnimations();
    }

    // ======== ALL SETTER GETTER FUNCTIONS ========
    void setName(const string& name) {
        info.name = name;
    }

    void takeDamage(float damage) {
        info.health -= damage;
        if (info.health < 0) info.health = 0;
    }


    string getName() const {
        return info.name;
    }

    void setHealth(int health) {
        info.health = health;
    }

    int getHealth() const {
        return info.health;
    }

    void addScore(int s) {
        info.score += s;
    }

    void setCoins(int coins) {
        info.coins = coins;
    }

    int getCoins() const {
        return info.coins;
    }

    void setScore(int score) {
        info.score = score;
    }

    int getScore() const {
        return info.score;
    }

    void increaseScore(int amount) {
        info.score += amount;
    }

    void reset() {
        info.health = 3;
        info.coins = 0;
        info.score = 0;
        velocityY = 0.0f;
        onGround = false;
        isJumping = false;
        facingRight = true;
        currentState = "idle";
        currentFrame = 0;
        animationTimer = 0.0f;
    }

    // ========== ANIMATION FUNCTIONS ==========
    void loadAnimations() {
        // Load idle animation (1 frame)
        idleFrameCount = 0;
        if (idleTextures[0].loadFromFile("Data/cuphead/idle/player.png")) {
            idleFrameCount = 1;
        }
        else {
            // Create placeholder
            Image img;
            img.create(50, 100, Color::Red);
            idleTextures[0].loadFromImage(img);
            idleFrameCount = 1;
        }

        // Load run animation (14 frames) - HARD CODED
        runFrameCount = 0;
        string runFiles[] = {
            "Data/cuphead/run/cuphead_run_shoot_0001.png",
            "Data/cuphead/run/cuphead_run_shoot_0002.png",
            "Data/cuphead/run/cuphead_run_shoot_0003.png",
            "Data/cuphead/run/cuphead_run_shoot_0004.png",
            "Data/cuphead/run/cuphead_run_shoot_0005.png",
            "Data/cuphead/run/cuphead_run_shoot_0006.png",
            "Data/cuphead/run/cuphead_run_shoot_0007.png",
            "Data/cuphead/run/cuphead_run_shoot_0008.png",
            "Data/cuphead/run/cuphead_run_shoot_0009.png",
            "Data/cuphead/run/cuphead_run_shoot_0010.png",
            "Data/cuphead/run/cuphead_run_shoot_0011.png",
            "Data/cuphead/run/cuphead_run_shoot_0012.png",
            "Data/cuphead/run/cuphead_run_shoot_0013.png",
            "Data/cuphead/run/cuphead_run_shoot_0014.png"
        };

        for (int i = 0; i < 14; i++) {
            if (runTextures[i].loadFromFile(runFiles[i])) {
                runFrameCount++;
            }
            else {
                // Create placeholder for missing frame
                Image img;
                img.create(50, 100, Color::Green);
                runTextures[i].loadFromImage(img);
                runFrameCount++;
            }
        }

        // Load jump animation (8 frames) - HARD CODED
        jumpFrameCount = 0;
        string jumpFiles[] = {
            "Data/cuphead/jump/cuphead_jump_0001.png",
            "Data/cuphead/jump/cuphead_jump_0002.png",
            "Data/cuphead/jump/cuphead_jump_0003.png",
            "Data/cuphead/jump/cuphead_jump_0004.png",
            "Data/cuphead/jump/cuphead_jump_0005.png",
            "Data/cuphead/jump/cuphead_jump_0006.png",
            "Data/cuphead/jump/cuphead_jump_0007.png",
            "Data/cuphead/jump/cuphead_jump_0008.png"
        };

        for (int i = 0; i < 8; i++) {
            if (jumpTextures[i].loadFromFile(jumpFiles[i])) {
                jumpFrameCount++;
            }
            else {
                // Create placeholder for missing frame
                Image img;
                img.create(50, 100, Color::Blue);
                jumpTextures[i].loadFromImage(img);
                jumpFrameCount++;
            }
        }

        // Set initial texture
        if (idleFrameCount > 0) {
            texture = idleTextures[0];
            sprite.setTexture(texture);
            textureLoaded = true;
            setOriginCenter();
        }
    }

    void updateAnimation(float deltaTime) {
        animationTimer += deltaTime;

        // Update animation frame based on state
        if (animationTimer >= GameConstants::FRAME_TIME) {
            animationTimer = 0.0f;

            // Get the appropriate texture array and frame count
            Texture* currentAnimation = nullptr;
            int frameCount = 0;

            if (currentState == "idle") {
                currentAnimation = idleTextures;
                frameCount = idleFrameCount;
            }
            else if (currentState == "run") {
                currentAnimation = runTextures;
                frameCount = runFrameCount;
            }
            else if (currentState == "jump") {
                currentAnimation = jumpTextures;
                frameCount = jumpFrameCount;
            }

            // Update frame if we have textures
            if (currentAnimation != nullptr && frameCount > 0) {
                currentFrame = (currentFrame + 1) % frameCount;
                texture = currentAnimation[currentFrame];
                sprite.setTexture(texture, true);
            }
        }
    }

    void setAnimationState(const string& state) {
        if (currentState != state) {
            currentState = state;
            currentFrame = 0;
            animationTimer = 0.0f;

            // Immediately update texture for new state
            if (currentState == "idle" && idleFrameCount > 0) {
                texture = idleTextures[0];
            }
            else if (currentState == "run" && runFrameCount > 0) {
                texture = runTextures[0];
            }
            else if (currentState == "jump" && jumpFrameCount > 0) {
                texture = jumpTextures[0];
            }

            sprite.setTexture(texture, true);
        }
    }

    void update(float deltaTime) override {
        updateAnimation(deltaTime);
    }

    // ========== ADDED PHYSICS FUNCTIONS ==========
    void applyGravity(float groundLevel) {
        // Apply gravity
        if (!onGround) {
            velocityY += GameConstants::GRAVITY;
            if (velocityY >= GameConstants::TERMINAL_VELOCITY)
                velocityY = GameConstants::TERMINAL_VELOCITY;
        }
        else {
            velocityY = 0;
        }

        // Get current position
        Coordinates pos = getPosition();

        // Update position
        pos.y += velocityY;

        // Ground collision check
        Rectangle bounds = getGlobalBounds();
        if (pos.y + bounds.height / 2 >= groundLevel) {
            pos.y = groundLevel - bounds.height / 2;
            onGround = true;
            velocityY = 0;
            isJumping = false;

            // Change to idle or run state when landing
            if (currentState == "jump") {
                setAnimationState("idle");
            }
        }
        else {
            onGround = false;
        }

        // Update position
        setPosition(pos.x, pos.y);
    }

    void jump() {
        if (onGround) {
            velocityY = GameConstants::JUMP_STRENGTH;
            isJumping = true;
            onGround = false;
            setAnimationState("jump");
        }
    }

    void moveHorizontal(float direction) {
        if (direction != 0) {
            // Update facing direction
            if (direction > 0) {
                facingRight = true;
            }
            else if (direction < 0) {
                facingRight = false;
            }

            // Update animation state if on ground
            if (onGround && currentState != "run") {
                setAnimationState("run");
            }

            Coordinates pos = getPosition();
            pos.x += direction * GameConstants::PLAYER_SPEED;

            // Keep player within screen bounds (considering origin is at center)
            Rectangle bounds = getGlobalBounds();
            float halfWidth = bounds.width / 2;
            if (pos.x - halfWidth < 0) pos.x = halfWidth;
            if (pos.x + halfWidth > GameConstants::WINDOW_WIDTH)
                pos.x = GameConstants::WINDOW_WIDTH - halfWidth;

            setPosition(pos.x, pos.y);
        }
        else if (onGround && currentState == "run") {
            // Stop running, go to idle
            setAnimationState("idle");
        }
    }

    bool getOnGround() const {
        return onGround;
    }

    void setOnGround(bool ground) {
        onGround = ground;
    }

    float getVelocityY() const {
        return velocityY;
    }

    bool isFacingRight() const {
        return facingRight;
    }
};

// =============================================
// ENEMY CLASS
// =============================================
class Enemy {
private:
    float groundLevelRef;

public:
    Sprite sprite;
    bool isAlive;
    int type;
    float health;
    Coordinates originalPosition;
    bool isGroundEnemy;
    Rectangle collisionBounds;

    Enemy(float x, float y, int enemyType, float groundLevel, bool groundEnemy = false) :
        isAlive(true), type(enemyType), health(1.0f),
        originalPosition(x, y), isGroundEnemy(groundEnemy), groundLevelRef(groundLevel) {

        Texture* enemyTexture = new Texture();
        string texturePath = "";

        // Assign texture based on enemy type
        switch (enemyType) {
        case 1:
            texturePath = "Data/blob/blob_melt_0001.png";
            break;
        case 2:
            texturePath = "Data/flowergrunt/flowergrunt_run_0001.png";
            break;
        case 3:
            texturePath = "Data/acron_machine/lv_1-1_acorn_machine_idle_0001.png";
            break;
        case 4:
            texturePath = "Data/acron_fly/acorn_fly_0001.png";
            break;
        case 5:
            texturePath = "Data/lobber/lobber_idle_0001.png";
            break;
        case 6:
            texturePath = "Data/mushroom/mushroom_attack_0001.png";
            break;
        default:
            texturePath = "Data/blob/blob_melt_0001.png";
            break;
        }

        if (enemyTexture->loadFromFile(texturePath)) {
            sprite.setTexture(*enemyTexture);
            // Set appropriate scale for each enemy type
            switch (enemyType) {
            case 1: sprite.setScale(0.5f, 0.5f); break;
            case 2: sprite.setScale(0.4f, 0.4f); break;
            case 3: sprite.setScale(0.3f, 0.3f); break;
            case 4: sprite.setScale(0.5f, 0.5f); break;
            case 5: sprite.setScale(0.6f, 0.6f); break;
            case 6: sprite.setScale(0.5f, 0.5f); break;
            }

            // Set origin to center for proper collision
            float leftVal = sprite.getLocalBounds().left;
            float topVal = sprite.getLocalBounds().top;
            float widthVal = sprite.getLocalBounds().width;
            float heightVal = sprite.getLocalBounds().height;
            Rectangle bounds(leftVal, topVal, widthVal, heightVal);
            sprite.setOrigin(bounds.width / 2, bounds.height / 2);

            // Adjust Y position for ground enemies
            float adjustedY = y;
            if (isGroundEnemy) {
                // Place on ground, considering sprite height
                adjustedY = groundLevelRef - (bounds.height * sprite.getScale().y / 2);
            }

            sprite.setPosition(x, adjustedY);

            // Set collision bounds
            collisionBounds = Rectangle(x - bounds.width / 2 * sprite.getScale().x,
                adjustedY - bounds.height / 2 * sprite.getScale().y,
                bounds.width * sprite.getScale().x,
                bounds.height * sprite.getScale().y);
        }
        else {
            // Fallback: create colored rectangle if texture fails to load
            cerr << "Failed to load enemy texture: " << texturePath << endl;

            // Create a rectangle shape as fallback
            sprite.setTextureRect({ 0, 0, 50, 50 });

            // Different colors for different enemy types
            switch (enemyType) {
            case 1: sprite.setColor(Color::Red); break;
            case 2: sprite.setColor(Color::Blue); break;
            case 3: sprite.setColor(Color::Green); break;
            case 4: sprite.setColor(Color::Magenta); break;
            case 5: sprite.setColor(Color(139, 69, 19)); break;
            case 6: sprite.setColor(Color(255, 140, 0)); break;
            }

            // Adjust Y position for ground enemies (fallback)
            float adjustedY = y;
            if (isGroundEnemy) {
                adjustedY = groundLevelRef - 25;
            }

            sprite.setPosition(x, adjustedY);
            collisionBounds = Rectangle(x - 25, adjustedY - 25, 50, 50);
        }
    }

    Rectangle getGlobalBounds() const {
        return collisionBounds;
    }

    Coordinates getPosition() const {
        float xVal = sprite.getPosition().x;
        float yVal = sprite.getPosition().y;
        return Coordinates(xVal, yVal);
    }

};

// =============================================
// PROJECTILE STRUCT
// =============================================
struct Projectile {
    CircleShape shape;
    Coordinates velocity;
    bool isActive;
    float lifetime;
    Coordinates worldPosition; // Store world position separately

    Projectile() : isActive(false), lifetime(3.0f) {
        shape.setRadius(10.0f);
        shape.setFillColor(Color::Cyan);
        shape.setOutlineColor(Color::White);
        shape.setOutlineThickness(2.0f);
    }

    void fire(float startX, float startY, float velX, float velY) {
        shape.setPosition(startX, startY);
        worldPosition = Coordinates(startX, startY);
        velocity = Coordinates(velX, velY);
        isActive = true;
        lifetime = 3.0f;
    }

    void update(float deltaTime) {
        if (!isActive) return;

        // Apply gravity to projectile
        velocity.y += 800.0f * deltaTime; // Projectile gravity

        // Update world position
        worldPosition.x += velocity.x * deltaTime;
        worldPosition.y += velocity.y * deltaTime;

        // Update shape position (will be converted to screen coordinates during rendering)
        shape.setPosition(worldPosition.x, worldPosition.y);

        // Reduce lifetime
        lifetime -= deltaTime;
        if (lifetime <= 0) {
            isActive = false;
        }
    }

    Rectangle getGlobalBounds() const {
        float radiusVal = shape.getRadius();

        float leftVal = worldPosition.x - radiusVal;
        float topVal = worldPosition.y - radiusVal;
        float widthVal = radiusVal * 2;
        float heightVal = radiusVal * 2;

        return Rectangle(leftVal, topVal, widthVal, heightVal);
    }

    Coordinates getWorldPosition() const {
        return worldPosition;
    }
};

// Custom array for projectiles
const int MAX_PROJECTILES = 10;
struct ProjectileArray {
    ProjectileArray() : size(0) {
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            data[i] = Projectile();
        }
    }

    Projectile* get(int index) {
        if (index < 0 || index >= size) return nullptr;
        return &data[index];
    }

    int getSize() const { return size; }

    void add() {
        if (size < MAX_PROJECTILES) {
            size++;
        }
    }

    void clear() { size = 0; }

private:
    Projectile data[MAX_PROJECTILES];
    int size;
};

// Custom View class replacement
class CustomView {
private:
    float centerX, centerY;
    float sizeX, sizeY;

public:
    CustomView() : centerX(0), centerY(0), sizeX(900), sizeY(650) {}

    CustomView(float centerX_, float centerY_, float sizeX_, float sizeY_)
        : centerX(centerX_), centerY(centerY_), sizeX(sizeX_), sizeY(sizeY_) {
    }

    void setCenter(float x, float y) {
        centerX = x;
        centerY = y;
    }

    void setSize(float width, float height) {
        sizeX = width;
        sizeY = height;
    }

    void move(float offsetX, float offsetY) {
        centerX += offsetX;
        centerY += offsetY;
    }

    float getCenterX() const { return centerX; }
    float getCenterY() const { return centerY; }
    float getSizeX() const { return sizeX; }
    float getSizeY() const { return sizeY; }

    // Get the visible area (left, top, width, height)
    Rectangle getViewport() const {
        float left = centerX - sizeX / 2;
        float top = centerY - sizeY / 2;
        return Rectangle(left, top, sizeX, sizeY);
    }

    // Transform world coordinates to screen coordinates
    Coordinates worldToScreen(float worldX, float worldY) const {
        Rectangle viewport = getViewport();
        float screenX = worldX - viewport.left;
        float screenY = worldY - viewport.top;
        return Coordinates(screenX, screenY);
    }

    // Transform screen coordinates to world coordinates
    Coordinates screenToWorld(float screenX, float screenY) const {
        Rectangle viewport = getViewport();
        float worldX = screenX + viewport.left;
        float worldY = screenY + viewport.top;
        return Coordinates(worldX, worldY);
    }
};

class GAME {

    Font font;
    Player player;
    Leaderboard leaderboard;

    RenderWindow window{ VideoMode(GameConstants::WINDOW_WIDTH, GameConstants::WINDOW_HEIGHT), "Game Window" };

    Music gameMusic;
    Clock game_Clock;

    void renderText(RenderWindow& window, const string& content, float x, float y, int size, Color color) {
        Text text;
        text.setFont(font);
        text.setString(content);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }

    string getName(RenderWindow& window) {
        Background bgGetNameScreen("Data/screens/entername.png");
        Button EnterButton(370.f, 380.f, 158.f, 50.f);
        string playerName;

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::MouseButtonPressed)
                    cout << event.mouseButton.x << " " << event.mouseButton.y << endl;

                if (EnterButton.isClicked(event, window)) {
                    if (!playerName.empty()) {
                        return playerName;
                    }
                }

                if (event.type == Event::Closed) {
                    window.close();
                }
                if (event.type == Event::TextEntered) {
                    if (event.text.unicode == '\b') {
                        if (!playerName.empty()) {
                            playerName.pop_back();
                        }
                    }
                    else if (event.text.unicode == '\r') {
                        return playerName;
                    }
                    else if (event.text.unicode < 128) {
                        char c = static_cast<char>(event.text.unicode);
                        if (c != ' ' && playerName.length() < 12) {
                            playerName += c;
                        }
                    }
                }
            }
            window.clear();
            bgGetNameScreen.draw(window);
            renderText(window, "Name: " + playerName, 300.f, 285.f, 30, Color::Black);
            window.display();
        }
        return playerName;
    }

    void showLeaderBoard(RenderWindow& window) {
        Background bgLeaderboard("Data/screens/leaderBoard.png");
        leaderboard.readLeaderboard();

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }

                if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                    return;
                }
            }
            window.clear();
            bgLeaderboard.draw(window);

            for (int i = 0; i < min(leaderboard.getSize(), 7); i++) {
                string entry = intToString(i + 1) + ". " +
                    leaderboard.getEntry(i).name + " - " +
                    intToString(leaderboard.getEntry(i).score);
                renderText(window, entry, 300.f, 220.f + i * 40.f, 30, Color::Black);
            }
            window.display();
        }
    }

    int levelSelection() {
        Background bgMainMenu("Data/screens/levelSelection.png");
        Button NormallevelButton(264.f, 236.f, 372.f, 68.f);
        Button bossLevelButton(264.f, 327.f, 372.f, 68.f);
        Button ExitButton(264.f, 418.f, 372.f, 68.f);

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::MouseButtonPressed)
                    cout << event.mouseButton.x << " " << event.mouseButton.y << endl;

                if (NormallevelButton.isClicked(event, window)) return 1;
                if (bossLevelButton.isClicked(event, window)) return 2;
                if (ExitButton.isClicked(event, window)) return -1;
                if (event.type == Event::Closed) window.close();
            }
            window.clear();
            bgMainMenu.draw(window);
            window.display();
        }
        return -1;
    }

    void normalLevelGameplay() {
        const float groundLevel = GameConstants::WINDOW_HEIGHT - 100;
        const float WORLD_WIDTH = 3000.0f;
        const float SCREEN_WIDTH = GameConstants::WINDOW_WIDTH;
        const float SCREEN_HEIGHT = GameConstants::WINDOW_HEIGHT;

        // Create custom camera view
        CustomView cameraView(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Load background texture
        Texture backgroundTexture;
        if (!backgroundTexture.loadFromFile("Data/screens/gameScreen.png")) {
            // Fallback if background doesn't exist
            backgroundTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        // Create background sprite that repeats
        Sprite backgroundSprite;
        backgroundSprite.setTexture(backgroundTexture);
        float bgTextureWidth = backgroundTexture.getSize().x;
        float bgRepeatCount = ceil(WORLD_WIDTH / bgTextureWidth);
        backgroundSprite.setTextureRect({ 0, 0, static_cast<int>(bgTextureWidth * bgRepeatCount),
                                        static_cast<int>(backgroundTexture.getSize().y) });
        backgroundSprite.setPosition(0, 0);
        backgroundSprite.setScale(1.0f, (float)SCREEN_HEIGHT / backgroundTexture.getSize().y);

        // Load ground texture
        Texture groundTexture;
        if (!groundTexture.loadFromFile("Data/ground.png")) {
            cerr << "Failed to load ground texture!" << endl;
            // Create fallback ground
            groundTexture.create(SCREEN_WIDTH, 100);
        }

        Sprite groundSprite;
        groundSprite.setTexture(groundTexture);
        float textureWidth = groundTexture.getSize().x;
        float repeatCount = ceil(WORLD_WIDTH / textureWidth);
        groundSprite.setTextureRect({ 0, 0, static_cast<int>(textureWidth * repeatCount),
                                    static_cast<int>(groundTexture.getSize().y) });
        groundSprite.setPosition(0, groundLevel - 2);
        groundSprite.setScale(1.0f, (float)(SCREEN_HEIGHT - groundLevel) / groundTexture.getSize().y);

        // Create enemies array
        EnemyArray enemies;

        // Create 8 enemies randomly placed across the map
        srand(static_cast<unsigned int>(time(0)));
        int totalEnemies = 8;

        // We'll place all enemies on ground
        float minX = 200.0f;
        float maxX = WORLD_WIDTH - 200.0f;

        // Array to store enemy world positions (FIXED POSITIONS)
        float enemyWorldPositions[MAX_ENEMIES];

        for (int i = 0; i < totalEnemies; i++) {
            bool validPosition = false;
            int attempts = 0;

            while (!validPosition && attempts < 100) {
                // Generate random X position
                float enemyX = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));

                // Check if this position is too close to existing enemies
                validPosition = true;
                float minDistance = 150.0f; // Minimum distance between enemies

                for (int j = 0; j < i; j++) {
                    if (fabs(enemyX - enemyWorldPositions[j]) < minDistance) {
                        validPosition = false;
                        break;
                    }
                }

                if (validPosition) {
                    // Random enemy type (1-6)
                    int enemyType = (rand() % 6) + 1;

                    // Create enemy on ground with FIXED position
                    Enemy* newEnemy = new Enemy(enemyX, groundLevel, enemyType, groundLevel, true);
                    enemies.add(newEnemy);

                    // Store FIXED world position (no movement)
                    enemyWorldPositions[i] = enemyX;

                    cout << "Created ground enemy type " << enemyType << " at FIXED X: " << enemyX << endl;
                    break;
                }

                attempts++;
            }
        }

        // Create projectiles array
        ProjectileArray projectiles;

        // Initialize projectiles
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            projectiles.add();
        }

        // =============================================
        // PLAYER SETUP
        // =============================================
        // Start player at center of screen
        float playerWorldX = SCREEN_WIDTH / 2;
        float playerWorldY = 150;

        player.setPosition(playerWorldX, playerWorldY);
        player.setOriginCenter();
        player.reset();

        const float PLAYER_SPEED = 300.0f;
        float playerVelocityX = 0;
        float playerVelocityY = 0;
        const float GRAVITY = 800.0f;
        const float JUMP_FORCE = -600.0f;
        bool isJumping = false;
        bool lastFacingRight = true;

        Clock deltaClock;
        game_Clock.restart();
        Clock levelTimeClock;
        Clock fireCooldownClock;
        const float FIRE_COOLDOWN = 0.3f;
        const float PROJECTILE_SPEED = 400.0f;
        const float PROJECTILE_ANGLE = -45.0f; // 45 degrees upward

        // Game state
        bool gameRunning = true;
        bool levelComplete = false;
        Clock invincibilityClock;
        const float INVINCIBILITY_TIME = 1.0f;
        bool isInvincible = false;

        while (window.isOpen() && gameRunning) {
            float deltaTime = deltaClock.restart().asSeconds();
            Event event;

            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();

                if (event.type == Event::MouseButtonPressed && !levelComplete) {
                    // Fire projectile on left mouse click
                    if (event.mouseButton.button == Mouse::Left && fireCooldownClock.getElapsedTime().asSeconds() >= FIRE_COOLDOWN) {
                        // Find inactive projectile
                        for (int i = 0; i < projectiles.getSize(); i++) {
                            Projectile* projectile = projectiles.get(i);
                            if (projectile && !projectile->isActive) {
                                // Use world position for projectile
                                float startX = playerWorldX;
                                float startY = playerWorldY - 30;

                                // Calculate velocity with angle for proper projectile motion
                                float angleRad = PROJECTILE_ANGLE * 3.14159f / 180.0f;
                                float velX = lastFacingRight ? PROJECTILE_SPEED * cos(angleRad) : -PROJECTILE_SPEED * cos(angleRad);
                                float velY = PROJECTILE_SPEED * sin(angleRad);

                                projectile->fire(startX, startY, velX, velY);
                                fireCooldownClock.restart();
                                cout << "Fired projectile from mouse! Velocity: (" << velX << ", " << velY << ")" << endl;
                                break;
                            }
                        }
                    }
                }

                if (event.type == Event::KeyPressed) {
                    if (!levelComplete && (event.key.code == Keyboard::Up || event.key.code == Keyboard::Space)) {
                        if (!isJumping) {
                            playerVelocityY = JUMP_FORCE;
                            isJumping = true;
                            cout << "Player jumped!" << endl;
                        }
                    }

                    // Fire projectile with F key
                    if (!levelComplete && event.key.code == Keyboard::F && fireCooldownClock.getElapsedTime().asSeconds() >= FIRE_COOLDOWN) {
                        for (int i = 0; i < projectiles.getSize(); i++) {
                            Projectile* projectile = projectiles.get(i);
                            if (projectile && !projectile->isActive) {
                                float startX = playerWorldX;
                                float startY = playerWorldY - 30;

                                // Calculate velocity with angle for proper projectile motion
                                float angleRad = PROJECTILE_ANGLE * 3.14159f / 180.0f;
                                float velX = lastFacingRight ? PROJECTILE_SPEED * cos(angleRad) : -PROJECTILE_SPEED * cos(angleRad);
                                float velY = PROJECTILE_SPEED * sin(angleRad);

                                projectile->fire(startX, startY, velX, velY);
                                fireCooldownClock.restart();
                                cout << "Fired projectile from F key! Velocity: (" << velX << ", " << velY << ")" << endl;
                                break;
                            }
                        }
                    }

                    if (event.key.code == Keyboard::Escape) {
                        gameRunning = false;
                        return;
                    }
                }
            }

            // =============================================
            // PLAYER MOVEMENT
            // =============================================
            if (!levelComplete) {
                float horizontalInput = 0.0f;
                bool isMoving = false;

                if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) {
                    horizontalInput = -1.0f;
                    isMoving = true;
                    lastFacingRight = false;
                }
                if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) {
                    horizontalInput = 1.0f;
                    isMoving = true;
                    lastFacingRight = true;
                }

                playerVelocityX = horizontalInput * PLAYER_SPEED;
                playerVelocityY += GRAVITY * deltaTime;

                // Update world position
                playerWorldX += playerVelocityX * deltaTime;
                playerWorldY += playerVelocityY * deltaTime;

                // Clamp player world X position
                float playerWidth = player.getSprite().getGlobalBounds().width;
                if (playerWorldX < playerWidth / 2) playerWorldX = playerWidth / 2;
                if (playerWorldX > WORLD_WIDTH - playerWidth / 2) playerWorldX = WORLD_WIDTH - playerWidth / 2;

                // Ground collision
                if (playerWorldY >= groundLevel) {
                    playerWorldY = groundLevel;
                    playerVelocityY = 0;
                    isJumping = false;
                }

                // Update camera to follow player
                float targetCameraX = playerWorldX;
                float targetCameraY = SCREEN_HEIGHT / 2; // Keep camera centered vertically

                // Smooth camera movement
                float currentCameraX = cameraView.getCenterX();
                float currentCameraY = cameraView.getCenterY();

                const float CAMERA_FOLLOW_SPEED = 5.0f;
                float newCameraX = currentCameraX + (targetCameraX - currentCameraX) * CAMERA_FOLLOW_SPEED * deltaTime;
                float newCameraY = currentCameraY + (targetCameraY - currentCameraY) * CAMERA_FOLLOW_SPEED * deltaTime;

                // Clamp camera to world boundaries
                if (newCameraX < SCREEN_WIDTH / 2) newCameraX = SCREEN_WIDTH / 2;
                if (newCameraX > WORLD_WIDTH - SCREEN_WIDTH / 2) newCameraX = WORLD_WIDTH - SCREEN_WIDTH / 2;

                cameraView.setCenter(newCameraX, newCameraY);

                // Calculate player screen position
                Coordinates screenPos = cameraView.worldToScreen(playerWorldX, playerWorldY);
                player.setPosition(screenPos.x, screenPos.y);

                // Update sprite flipping
                Sprite& playerSprite = player.getSprite();
                float x_currentScale = playerSprite.getScale().x;
                float y_currentScale = playerSprite.getScale().y;

                if (lastFacingRight) {
                    playerSprite.setScale(fabs(x_currentScale), y_currentScale);
                }
                else {
                    playerSprite.setScale(-fabs(x_currentScale), y_currentScale);
                }

                // Update animation state
                if (isJumping) {
                    player.setAnimationState("jump");
                }
                else if (isMoving) {
                    player.setAnimationState("run");
                }
                else {
                    player.setAnimationState("idle");
                }

                player.update(deltaTime);

                // =============================================
                // NO ENEMY MOVEMENT - FIXED POSITIONS
                // =============================================
                // Enemies stay at their fixed positions, no movement code needed

                // Just update collision bounds for enemies at their fixed positions
                for (int i = 0; i < enemies.getSize(); i++) {
                    Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                    if (enemy && enemy->isAlive) {
                        // Set enemy sprite to its fixed world position (for collision)
                        float enemyWorldY = groundLevel - enemy->sprite.getGlobalBounds().height / 2;
                        enemy->sprite.setPosition(enemyWorldPositions[i], enemyWorldY);

                        // Update collision bounds
                        FloatRect spriteBounds = enemy->sprite.getGlobalBounds();
                        enemy->collisionBounds = Rectangle(spriteBounds.left, spriteBounds.top,
                            spriteBounds.width, spriteBounds.height);
                    }
                }

                // =============================================
                // UPDATE PROJECTILES
                // =============================================
                for (int i = 0; i < projectiles.getSize(); i++) {
                    Projectile* projectile = projectiles.get(i);
                    if (projectile && projectile->isActive) {
                        projectile->update(deltaTime);

                        // Check if projectile is out of bounds (world coordinates)
                        Coordinates projPos = projectile->getWorldPosition();

                        if (projPos.x < 0 || projPos.x > WORLD_WIDTH || projPos.y > SCREEN_HEIGHT) {
                            projectile->isActive = false;
                            cout << "Projectile " << i << " out of bounds, deactivated" << endl;
                        }
                    }
                }

                // =============================================
                // COLLISION DETECTION (using world coordinates)
                // =============================================
                Rectangle playerBounds = player.getGlobalBounds();
                // Adjust player bounds to world coordinates for collision
                float playerWorldBoundsLeft = playerWorldX - playerBounds.width / 2;
                float playerWorldBoundsTop = playerWorldY - playerBounds.height / 2;
                Rectangle playerWorldBounds(playerWorldBoundsLeft, playerWorldBoundsTop,
                    playerBounds.width, playerBounds.height);

                if (!isInvincible) {
                    for (int i = 0; i < enemies.getSize(); i++) {
                        Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                        if (enemy && enemy->isAlive) {
                            // Get enemy bounds from sprite (fixed position)
                            FloatRect spriteBounds = enemy->sprite.getGlobalBounds();
                            Rectangle enemyBounds(spriteBounds.left, spriteBounds.top,
                                spriteBounds.width, spriteBounds.height);

                            if (playerWorldBounds.intersects(enemyBounds)) {
                                player.takeDamage(1);
                                cout << "Player hit! Health: " << player.getHealth() << endl;

                                enemy->isAlive = false;
                                isInvincible = true;
                                invincibilityClock.restart();
                                player.addScore(50);

                                // Push player back
                                if (playerWorldX < enemyWorldPositions[i]) {
                                    playerWorldX -= 50;
                                }
                                else {
                                    playerWorldX += 50;
                                }

                                break;
                            }
                        }
                    }
                }

                // Projectile vs Enemy collision
                for (int i = 0; i < projectiles.getSize(); i++) {
                    Projectile* projectile = projectiles.get(i);
                    if (projectile && projectile->isActive) {
                        Rectangle projectileBounds = projectile->getGlobalBounds();

                        for (int j = 0; j < enemies.getSize(); j++) {
                            Enemy* enemy = static_cast<Enemy*>(enemies.get(j));
                            if (enemy && enemy->isAlive) {
                                // Get enemy bounds from sprite (fixed position)
                                FloatRect spriteBounds = enemy->sprite.getGlobalBounds();
                                Rectangle enemyBounds(spriteBounds.left, spriteBounds.top,
                                    spriteBounds.width, spriteBounds.height);

                                if (projectileBounds.intersects(enemyBounds)) {
                                    enemy->health -= 0.5f;

                                    if (enemy->health <= 0) {
                                        enemy->isAlive = false;
                                        player.addScore(100);
                                        cout << "Enemy destroyed! Score: " << player.getScore() << endl;
                                    }

                                    projectile->isActive = false;
                                    cout << "Projectile hit enemy " << j << "! Enemy health: " << enemy->health << endl;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Update invincibility
                if (isInvincible && invincibilityClock.getElapsedTime().asSeconds() >= INVINCIBILITY_TIME) {
                    isInvincible = false;
                }

                // =============================================
                // CHECK LEVEL COMPLETION
                // =============================================
                int aliveEnemies = 0;
                for (int i = 0; i < enemies.getSize(); i++) {
                    Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                    if (enemy && enemy->isAlive) aliveEnemies++;
                }

                if (aliveEnemies == 0 && !levelComplete) {
                    levelComplete = true;
                    levelTimeClock.restart();
                    cout << "Level Complete! All enemies defeated!" << endl;
                }

                // =============================================
                // GAME OVER CHECK
                // =============================================
                if (player.getHealth() <= 0) {
                    cout << "Game Over! Final Score: " << player.getScore() << endl;

                    // Show game over screen for 2 seconds
                    Clock gameOverClock;
                    while (gameOverClock.getElapsedTime().asSeconds() < 2.0f) {
                        window.clear();
                        renderText(window, "GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 48, Color::Red);
                        renderText(window, "Score: " + to_string(player.getScore()),
                            SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 20, 32, Color::White);
                        window.display();

                        Event gameOverEvent;
                        while (window.pollEvent(gameOverEvent)) {
                            if (gameOverEvent.type == Event::Closed) {
                                window.close();
                                return;
                            }
                        }
                    }
                    gameRunning = false;
                    return;
                }
            }

            // =============================================
            // LEVEL COMPLETE SCREEN
            // =============================================
            if (levelComplete) {
                if (levelTimeClock.getElapsedTime().asSeconds() >= 2.0f) {
                    cout << "Returning to menu..." << endl;
                    return;
                }
            }

            // =============================================
            // RENDERING (with custom View)
            // =============================================
            window.clear();

            // Get the visible area from camera
            Rectangle viewport = cameraView.getViewport();

            // Draw background (position it relative to camera)
            float bgScreenX = 0 - viewport.left;
            backgroundSprite.setPosition(bgScreenX, 0);
            window.draw(backgroundSprite);

            // Draw ground
            float groundScreenX = 0 - viewport.left;
            groundSprite.setPosition(groundScreenX, groundLevel - 2);
            window.draw(groundSprite);

            // Draw enemies with camera transformation (FIXED positions)
            for (int i = 0; i < enemies.getSize(); i++) {
                Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                if (enemy && enemy->isAlive) {
                    // Convert FIXED world position to screen position for drawing
                    float enemyWorldY = groundLevel - enemy->sprite.getGlobalBounds().height / 2;
                    Coordinates screenPos = cameraView.worldToScreen(enemyWorldPositions[i], enemyWorldY);

                    // Set sprite position for rendering
                    enemy->sprite.setPosition(screenPos.x, screenPos.y);
                    window.draw(enemy->sprite);

                    // Draw enemy FIXED world position text above enemy
                    string enemyPosText = "E" + to_string(i) + " (FIXED): " +
                        to_string(static_cast<int>(enemyWorldPositions[i])) + "," +
                        to_string(static_cast<int>(enemyWorldY));
                    //renderText(window, enemyPosText, screenPos.x - 40, screenPos.y - 50, 12, Color::Yellow);

                    // Draw enemy health bar
                    if (enemy->health < 1.0f) {
                        RectangleShape healthBar({ 50, 5 });
                        healthBar.setPosition(screenPos.x - 25, screenPos.y - 60);
                        healthBar.setFillColor(Color::Red);
                        healthBar.setOutlineColor(Color::Black);
                        healthBar.setOutlineThickness(1);

                        RectangleShape healthFill({ 50 * enemy->health, 5 });
                        healthFill.setPosition(screenPos.x - 25, screenPos.y - 60);
                        healthFill.setFillColor(Color::Green);

                        window.draw(healthBar);
                        window.draw(healthFill);
                    }
                }
            }

            // Draw projectiles with camera transformation
            for (int i = 0; i < projectiles.getSize(); i++) {
                Projectile* projectile = projectiles.get(i);
                if (projectile && projectile->isActive) {
                    Coordinates projWorldPos = projectile->getWorldPosition();
                    Coordinates screenPos = cameraView.worldToScreen(projWorldPos.x, projWorldPos.y);
                    projectile->shape.setPosition(screenPos.x, screenPos.y);
                    window.draw(projectile->shape);

                    // Draw projectile world position
                    string projPosText = "P: " + to_string(static_cast<int>(projWorldPos.x)) + "," +
                        to_string(static_cast<int>(projWorldPos.y));
                    //renderText(window, projPosText, screenPos.x - 20, screenPos.y - 20, 10, Color::Cyan);

                }
            }

            // Draw player with invincibility flashing effect
            if (!levelComplete) {
                if (!isInvincible || (int)(invincibilityClock.getElapsedTime().asSeconds() * 10) % 2 == 0) {
                    // Player is already at correct screen position
                    player.draw(window);
                }
            }

            // =============================================
            // DEBUG INFO - WORLD POSITIONS
            // =============================================
            // Draw player world position
            // string playerWorldPosText = "Player World: " + 
            //                            to_string(static_cast<int>(playerWorldX)) + "," + 
            //                            to_string(static_cast<int>(playerWorldY));
            // renderText(window, playerWorldPosText, 10.f, 70.f, 18, Color::Green);

            // // Draw camera/viewport info
            // string cameraInfo = "Camera: " + 
            //                    to_string(static_cast<int>(cameraView.getCenterX())) + "," + 
            //                    to_string(static_cast<int>(cameraView.getCenterY()));
            // renderText(window, cameraInfo, 10.f, 100.f, 18, Color::Magenta);

            // string viewportInfo = "Viewport: " + 
            //                      to_string(static_cast<int>(viewport.left)) + " to " + 
            //                      to_string(static_cast<int>(viewport.left + viewport.width));
            // renderText(window, viewportInfo, 10.f, 130.f, 18, Color::Cyan);

            // Draw projectile info
            int activeProjectiles = 0;
            for (int i = 0; i < projectiles.getSize(); i++) {
                Projectile* projectile = projectiles.get(i);
                if (projectile && projectile->isActive) activeProjectiles++;
            }
            // string projectileInfo = "Active Projectiles: " + to_string(activeProjectiles) + "/" + to_string(MAX_PROJECTILES);
            // renderText(window, projectileInfo, 10.f, 160.f, 16, Color::Cyan);

            // // Draw firing angle info
            // string angleInfo = "Firing Angle: " + to_string(PROJECTILE_ANGLE) + "°";
            // renderText(window, angleInfo, 10.f, 185.f, 16, Color::Cyan);

            // // Draw enemy positions list (FIXED)
            // int yOffset = 210;
            // renderText(window, "Enemy FIXED Positions:", 10.f, yOffset, 16, Color::White);
            // yOffset += 20;

            for (int i = 0; i < enemies.getSize(); i++) {
                Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                if (enemy) {
                    float enemyWorldY = groundLevel - enemy->sprite.getGlobalBounds().height / 2;
                    string enemyInfo = "E" + to_string(i) + ": " +
                        to_string(static_cast<int>(enemyWorldPositions[i])) + "," +
                        to_string(static_cast<int>(enemyWorldY)) +
                        " (Type: " + to_string(enemy->type) +
                        ", Health: " + to_string(enemy->health) +
                        ", Alive: " + (enemy->isAlive ? "Yes" : "No") + ")";
                    // renderText(window, enemyInfo, 10.f, yOffset, 14, 
                    //           enemy->isAlive ? Color::Yellow : Color::Red);
                    // yOffset += 18;
                }
            }

            // =============================================
            // UI (no camera transformation)
            // =============================================
            renderText(window, "Score: " + to_string(player.getScore()), 765.f, 30.f, 24, Color::White);
            renderText(window, "Health: " + to_string(player.getHealth()), 130.f, 30.f, 24, Color::White);
            renderText(window, "Time: " + to_string(static_cast<int>(game_Clock.getElapsedTime().asSeconds())),
                471.f, 30.f, 24, Color::White);

            if (!levelComplete) {
                renderText(window, "Controls: F/Mouse - Shoot, Space - Jump", 10.f, 10.f, 18, Color::White);
                //renderText(window, "Fire Cooldown: " + to_string(fireCooldownClock.getElapsedTime().asSeconds()), 
                //          10.f, 35.f, 16, Color::Yellow);

                // Draw enemy counter
                int aliveEnemies = 0;
                for (int i = 0; i < enemies.getSize(); i++) {
                    Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
                    if (enemy && enemy->isAlive) aliveEnemies++;
                }
                renderText(window, "Enemies: " + to_string(aliveEnemies) + "/8", 10.f, 55.f, 18, Color::White);
            }

            // =============================================
            // LEVEL COMPLETE / GAME OVER SCREEN
            // =============================================
            if (levelComplete) {
                // Semi-transparent overlay
                RectangleShape overlay({ SCREEN_WIDTH, SCREEN_HEIGHT });
                overlay.setFillColor(Color(0, 0, 0, 150));
                window.draw(overlay);

                renderText(window, "LEVEL COMPLETE!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 60, 48, Color::Green);
                renderText(window, "Score: " + to_string(player.getScore()),
                    SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2, 32, Color::White);
                renderText(window, "Returning to menu...",
                    SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 60, 24, Color::Yellow);
            }

            window.display();
        }

        // Clean up enemies
        for (int i = 0; i < enemies.getSize(); i++) {
            Enemy* enemy = static_cast<Enemy*>(enemies.get(i));
            if (enemy) {
                delete enemy;
            }
        }
    }

    void mainMenu() {
        Background bgMainMenu("Data/screens/mainmenu.png");
        Button PlayButton(264.f, 236.f, 372.f, 68.f);
        Button LeaderboardButton(264.f, 327.f, 372.f, 68.f);
        Button ExitButton(264.f, 418.f, 372.f, 68.f);

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::MouseButtonPressed)
                    cout << event.mouseButton.x << " " << event.mouseButton.y << endl;

                if (PlayButton.isClicked(event, window)) {
                    int level = levelSelection();
                    if (level == -1) continue;

                    if (level == 1) {
                        normalLevelGameplay();
                        leaderboard.addToLeaderboard(player.getName(), player.getScore());
                    }
                    else if (level == 2) {
                        cout << "Playing Boss Level..." << endl;
                    }
                }

                if (LeaderboardButton.isClicked(event, window)) {
                    showLeaderBoard(window);
                }

                if (ExitButton.isClicked(event, window)) {
                    window.close();
                }

                if (event.type == Event::Closed) {
                    window.close();
                }
            }
            window.clear();
            bgMainMenu.draw(window);
            window.display();
        }
    }

public:
    GAME() {
        if (!font.loadFromFile("fonts/font.ttf")) {
            cout << "Failed to load font." << endl;
        }

        // Music setup
        if (!gameMusic.openFromFile("Data/mus.ogg")) {
            cout << "Failed to load music!" << endl;
        }
        gameMusic.setVolume(20);
        gameMusic.play();
        gameMusic.setLoop(true);

        // Load player animations
        player.setScale(0.75f, 0.75f);

        //Full flow
        player.setName(getName(window));
        mainMenu();
    }

    ~GAME() {
        gameMusic.stop();
    }
};

int main() {
    GAME game;
    return 0;
}