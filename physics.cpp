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
    std::vector<Object> wallColliders;
    std::vector<Object> pickupColliders;
    std::vector<sf::RectangleShape> walls;
    std::vector<sf::CircleShape> pickups;

    // Create game window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Collision Viewer", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // Create player object
    Object A = { Vec2(300, 250), { Vec2(-50, -50), Vec2(50, 50) } }; // Playable box
    sf::RectangleShape boxA(sf::Vector2f(100, 100));
    boxA.setPosition(toSF(A.pos));
    boxA.setOutlineThickness(2);
    boxA.setOutlineColor(sf::Color::Green);
    boxA.setFillColor(sf::Color::Transparent);


    // Create wall objects
    Object B = { Vec2(420, 270), { Vec2(-50, -50), Vec2(50, 50) } };
    Object C = { Vec2(220, 120), { Vec2(-50, -50), Vec2(50, 50) } };
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

    // Set up wall objects list
    // Group wall colliders
    wallColliders.push_back(B);
    wallColliders.push_back(C);
    // Group wall textures
    walls.push_back(boxB);
    walls.push_back(boxC);


    // Create pickup objects
    Object PickA = { Vec2(540, 220), { Vec2(-50, -50), Vec2(50, 50) } };
    Object PickB = { Vec2(140, 120), { Vec2(-50, -50), Vec2(50, 50) } };
    sf::CircleShape pickupA(30.f, 30.f);
    pickupA.setPosition(toSF(PickA.pos));
    pickupA.setOutlineThickness(1);
    pickupA.setOutlineColor(sf::Color::Green);
    pickupA.setFillColor(sf::Color::Green);
    sf::CircleShape pickupB(30.f, 30.f);
    pickupB.setPosition(toSF(PickB.pos));
    pickupB.setOutlineThickness(1);
    pickupB.setOutlineColor(sf::Color::Green);
    pickupB.setFillColor(sf::Color::Green);
    
    // Set up pickup objects list
    // Group pickup colliders
    pickupColliders.push_back(PickA);
    pickupColliders.push_back(PickB);
    // Group pickup textures
    pickups.push_back(pickupA);
    pickups.push_back(pickupB);

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
        for (auto& obj : wallColliders) {
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
        for (auto& obj : wallColliders) {
            Manifold m = { &A, &obj };
            if (AABBvsAABB(&m)) {
                A.pos.y = originalY;
                break;
            }
        }

        // Delete pickup if collided with
        for (auto it = pickupColliders.begin(); it != pickupColliders.end(); ) {
            Manifold m = {&A, &(*it)};
            if (AABBvsAABB(&m)) {
                pickups.erase(std::remove_if(pickups.begin(), pickups.end(),
                  [&](const sf::CircleShape& shape) {
                      return shape.getPosition() == toSF(it->pos);
                  }), pickups.end());
                it = pickupColliders.erase(it);
                break;
            } else {
                ++it;
            }
        }

        // Set up window
        window.clear();
        // Draw walls
        for (const auto& wall : walls) {
          window.draw(wall);
        }
        // Draw pickups
        for (const auto& pickup : pickups) {
          window.draw(pickup);
        }
        // Draw playable box
        boxA.setPosition(toSF(A.pos));
        window.draw(boxA);

        window.display();
    }

    return 0;
}