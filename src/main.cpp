#include <SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <SDL_ttf.h>
#include <sstream>
#include <SDL_image.h>




const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SNAKE_SIZE = 20;
const int FPS = 10;
const int FRAME_DELAY = 1000 / FPS;  



enum class Direction {
    NONE, UP, DOWN, LEFT, RIGHT
};

struct Segment {
    int x;
    int y;
};

enum GameState {
    TITLE,
    PLAYING
};

class Snake {
public:
    SDL_Texture* appleTexture; 

    Snake(SDL_Texture* appleTex) : appleTexture(appleTex), direction(Direction::NONE), grow(false), gameOver(false), score(0) {
        segments.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
        placeApple();
    }

    int getScore() const {
        return score;
    }

    bool isGameOver() const {
        return gameOver;
    }

    void handleEvent(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {  
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    if (direction != Direction::DOWN) direction = Direction::UP;
                    break;
                case SDLK_DOWN:
                    if (direction != Direction::UP) direction = Direction::DOWN;
                    break;
                case SDLK_LEFT:
                    if (direction != Direction::RIGHT) direction = Direction::LEFT;
                    break;
                case SDLK_RIGHT:
                    if (direction != Direction::LEFT) direction = Direction::RIGHT;
                    break;
            }
        }
    }

    void move() {
        if (direction == Direction::NONE) return;

        Segment new_head = segments.front();
        switch (direction) {
            case Direction::UP:    new_head.y -= SNAKE_SIZE; break;
            case Direction::DOWN:  new_head.y += SNAKE_SIZE; break;
            case Direction::LEFT:  new_head.x -= SNAKE_SIZE; break;
            case Direction::RIGHT: new_head.x += SNAKE_SIZE; break;
        }

        // Checks to see if snake hits borders
        if (new_head.x < 0 || new_head.y < 0 || new_head.x >= SCREEN_WIDTH || new_head.y >= SCREEN_HEIGHT) {
            gameOver = true;
            return;
        }

        // Check to see if snake hits itself
        for (auto segment = segments.begin() + 1; segment != segments.end(); ++segment) {
            if (new_head.x == segment->x && new_head.y == segment->y) {
                gameOver = true;
                return;
            }
        }

        if (new_head.x == apple.x && new_head.y == apple.y) {
            grow = true;
            placeApple();
            score += 10;  
        } else {
            grow = false;
        }

        segments.push_front(new_head);
        if (!grow) {
            segments.pop_back();
        }
    }

    void draw(SDL_Renderer* renderer) {
        // Draw the snake
        for (auto& segment : segments) {
            SDL_Rect fillRect = {segment.x, segment.y, SNAKE_SIZE, SNAKE_SIZE};
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &fillRect);
        }

        // Draw the apple using the texture
        SDL_Rect appleRect = {apple.x, apple.y, SNAKE_SIZE, SNAKE_SIZE};
        SDL_RenderCopy(renderer, appleTexture, NULL, &appleRect);
    }

    void placeApple() {
        int xMax = SCREEN_WIDTH / SNAKE_SIZE;
        int yMax = SCREEN_HEIGHT / SNAKE_SIZE;
        apple.x = (rand() % xMax) * SNAKE_SIZE;
        apple.y = (rand() % yMax) * SNAKE_SIZE;
    }

private:
    std::deque<Segment> segments;
    Direction direction;
    bool grow;
    bool gameOver;
    Segment apple;
    int score;
};

SDL_Texture* loadTexture(const std::string &path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void drawFood(SDL_Renderer* renderer, SDL_Texture* appleTexture, const SDL_Rect& appleRect) {
    SDL_RenderCopy(renderer, appleTexture, NULL, &appleRect);
}

void drawGrid(SDL_Renderer* renderer) {
    int tileSize = SNAKE_SIZE;  
    SDL_Color color1 = {230, 230, 230, 255};  
    SDL_Color color2 = {210, 210, 210, 255}; 

    for (int y = 0; y < SCREEN_HEIGHT; y += tileSize) {
        for (int x = 0; x < SCREEN_WIDTH; x += tileSize) {
            SDL_Rect rect = {x, y, tileSize, tileSize};
            if ((x / tileSize + y / tileSize) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, color1.r, color1.g, color1.b, color1.a);
            } else {
                SDL_SetRenderDrawColor(renderer, color2.r, color2.g, color2.b, color2.a);
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void showGameOverScreen(SDL_Renderer* renderer, int finalScore, bool& restart) {
    TTF_Font* font = TTF_OpenFont("fonts/Merriweather-Bold.ttf", 24);  
    if (!font) {
      std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
      return;
    }

    SDL_Color textColor = {255, 255, 255, 255};  
    SDL_Surface* surfaceMessage;
    SDL_Texture* message;

    bool done = false;
    SDL_Event e;

    while (!done) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                done = true;
                restart = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    done = true;
                    restart = true;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                    restart = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  
        SDL_RenderClear(renderer);

        //Render "Game Over" text
        surfaceMessage = TTF_RenderText_Solid(font, "Game Over", textColor);
        message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect message_rect = {SCREEN_WIDTH / 2 - surfaceMessage->w / 2, SCREEN_HEIGHT / 2 - surfaceMessage->h / 2 - 20, surfaceMessage->w, surfaceMessage->h};
        SDL_RenderCopy(renderer, message, NULL, &message_rect);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(message);

        // Render score text
        std::string scoreText = "Score: " + std::to_string(finalScore);
        surfaceMessage = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect score_rect = {SCREEN_WIDTH / 2 - surfaceMessage->w / 2, SCREEN_HEIGHT / 2 + 10, surfaceMessage->w, surfaceMessage->h};
        SDL_RenderCopy(renderer, message, NULL, &score_rect);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(message);

        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);  
}

void playGame(SDL_Renderer* renderer, SDL_Texture* appleTexture) {

    bool restart = false;
    do {
       
        Snake snake(appleTexture);

        snake.placeApple();  

        Uint32 start_time;
        bool quit = false;
        SDL_Event e;
        
        while (!quit && !snake.isGameOver()) {
            start_time = SDL_GetTicks();
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    restart = false;
                }
                snake.handleEvent(e);
            }
            snake.move();
            //Sets the background to white, then draws the grid
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);  
            SDL_RenderClear(renderer);

            drawGrid(renderer);  
            snake.draw(renderer);  

            SDL_RenderPresent(renderer);

            //Incharge of how fast the game runs
            int frame_time = SDL_GetTicks() - start_time;
            if (frame_time < FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - frame_time);
            }
        }
        if (snake.isGameOver()) {
            showGameOverScreen(renderer, snake.getScore(), restart);
        }
    } while (restart);
}

void showTitleScreen(SDL_Renderer* renderer) {
    TTF_Font* font = TTF_OpenFont("fonts/Merriweather-Bold.ttf", 24);  
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Color textColor = {255, 255, 255, 255};  
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Rect rect;

    bool done = false;
    SDL_Event e;

    while (!done) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                exit(0);
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    done = true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  
        SDL_RenderClear(renderer);

        // Title 
        surface = TTF_RenderText_Solid(font, "SNAKE", textColor);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect = {SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 3, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        // Instructions
        surface = TTF_RenderText_Solid(font, "Use arrow keys to move. Eat fruit to gain 10 points.", textColor);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect = {SCREEN_WIDTH / 2 - surface->w / 2, SCREEN_HEIGHT / 2, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        surface = TTF_RenderText_Solid(font, "Press Enter to start.", textColor);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect = {SCREEN_WIDTH / 2 - surface->w / 2, 2 * SCREEN_HEIGHT / 3, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);  
}

int main(int argc, char* argv[]) {
    srand(time(NULL));  
    //Error checking code
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window Creation Error, SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        std::cout << "Rendering Error, SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Texture* appleTexture = loadTexture("assets/apple.png", renderer);  
    if (!appleTexture) {
        std::cerr << "Failed to load apple texture!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    showTitleScreen(renderer);
    playGame(renderer, appleTexture);

    SDL_DestroyTexture(appleTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}