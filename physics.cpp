#include <iostream>
#include <cmath>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

// The basic layout for this code was made following a tutorial at https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t

struct Vec2 {
    float x, y;
    Vec2(float x_=0, float y_=0) : x(x_), y(y_) {}
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    float abs() const { return std::sqrt(x*x + y*y); }
};

struct AABB {
    Vec2 min;
    Vec2 max;
};

struct Object {
    Vec2 pos;
    AABB aabb;
};

struct Manifold {
    Object *A, *B;
    Vec2 normal;
    float penetration;
};

// Compare objects
bool AABBvsAABB( Manifold *m )
{
  // Setup a couple pointers to each object 
  Object *A = m->A;
  Object *B = m->B;
 
  // Vector from A to B 
  Vec2 n = B->pos - A->pos;
 
  AABB abox = A->aabb;
  AABB bbox = B->aabb;
 
  // Calculate half extents along x axis for each object 
  float a_extent = (abox.max.x - abox.min.x) / 2;
  float b_extent = (bbox.max.x - bbox.min.x) / 2;
 
  // Calculate overlap on x axis 
  float x_overlap = a_extent + b_extent - abs( n.x );
 
  // SAT test on x axis 
  if(x_overlap > 0)
  {
    // Calculate half extents along x axis for each object 
    float a_extent = (abox.max.y - abox.min.y) / 2;
    float b_extent = (bbox.max.y - bbox.min.y) / 2;
 
    // Calculate overlap on y axis 
    float y_overlap = a_extent + b_extent - abs( n.y );
 
    // SAT test on y axis 
    if(y_overlap > 0)
    {
      // Find out which axis is axis of least penetration 
      if(x_overlap > y_overlap)
      {
        // Point towards B knowing that n points from A to B 
        if(n.x < 0)
          m->normal = Vec2( -1, 0 );
        else
          m->normal = Vec2( 0, 0 );
        m->penetration = x_overlap;
        return true;
      }
      else
      {
        // Point toward B knowing that n points from A to B 
        if(n.y < 0)
          m->normal = Vec2( 0, -1 );
        else
          m->normal = Vec2( 0, 1 );
        m->penetration = y_overlap;
        return true;
      }
      return false;
    }
    return false;
  }
  return false;
};

// Function to convert vector
sf::Vector2f toSF(const Vec2& v) {
    return sf::Vector2f(v.x, v.y);
}

// Function to get dot product 
float Dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}


// To test the physics engine
int main()
{
    std::vector<Object> colliders;
    std::vector<sf::RectangleShape> walls;

    // Create game window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Collision Viewer", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // Create box objects
    Object A = { Vec2(300, 250), { Vec2(-50, -50), Vec2(50, 50) } }; // Playable box
    Object B = { Vec2(420, 270), { Vec2(-50, -50), Vec2(50, 50) } };
    Object C = { Vec2(220, 120), { Vec2(-50, -50), Vec2(50, 50) } };
    sf::RectangleShape boxA(sf::Vector2f(100, 100));
    boxA.setPosition(toSF(A.pos));
    boxA.setOutlineThickness(2);
    boxA.setOutlineColor(sf::Color::Green);
    boxA.setFillColor(sf::Color::Transparent);
    sf::RectangleShape boxB(sf::Vector2f(100, 100));
    boxB.setPosition(toSF(B.pos));
    boxB.setOutlineThickness(2);
    boxB.setOutlineColor(sf::Color::White);
    boxB.setFillColor(sf::Color::Transparent);
    sf::RectangleShape boxC(sf::Vector2f(100, 100));
    boxC.setPosition(toSF(C.pos));
    boxC.setOutlineThickness(2);
    boxC.setOutlineColor(sf::Color::White);
    boxC.setFillColor(sf::Color::Transparent);
    
    // Group colliders and textures
    // Make these objects members of the collidable objects
    colliders.push_back(B);
    colliders.push_back(C);
    // Make these objects walls
    walls.push_back(boxB);
    walls.push_back(boxC);

    // Ensures window closes properly when closed
    const auto onClose = [&window](const sf::Event::Closed&) {
        window.close();
    };

    while (window.isOpen())
    {
      window.handleEvents(onClose);

      float speed = 5.f;
      Vec2 movement(0.f, 0.f);

      // Move playable box with wasd
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) movement.y -= speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) movement.y += speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) movement.x -= speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) movement.x += speed;

        // Try movement
        Vec2 originalPos = A.pos;
        // --- X axis ---
        float originalX = A.pos.x;
        A.pos.x += movement.x;
        // Stop if colliding with object
        for (auto& obj : colliders) {
            Manifold m = { &A, &obj };
            if (AABBvsAABB(&m)) {
                A.pos.x = originalX;
                break;
            }
        }
        // --- Y axis ---
        float originalY = A.pos.y;
        A.pos.y += movement.y;
        // Stop if colliding with object
        for (auto& obj : colliders) {
            Manifold m = { &A, &obj };
            if (AABBvsAABB(&m)) {
                A.pos.y = originalY;
                break;
            }
        }

        // Set up window
        window.clear();
        // Draw walls
        for (const auto& wall : walls) {
          window.draw(wall);
        }
        // Draw playable box
        boxA.setPosition(toSF(A.pos));
        window.draw(boxA);

        window.display();
    }

    return 0;
}