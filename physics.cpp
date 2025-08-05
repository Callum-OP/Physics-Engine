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

// To test the physics engine
int main()
{
    // Create game window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "AABB Collision Viewer", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    Object A = { Vec2(300, 250), { Vec2(-50, -50), Vec2(50, 50) } };
    Object B = { Vec2(400, 250), { Vec2(-50, -50), Vec2(50, 50) } };

    sf::RectangleShape boxA(sf::Vector2f(100, 100));  // width = max - min
    boxA.setPosition(toSF(A.pos));
    boxA.setOutlineThickness(2);

    sf::RectangleShape boxB(sf::Vector2f(100, 100));
    boxB.setPosition(toSF(B.pos));
    boxB.setOutlineThickness(2);

    // Ensures window closes properly when closed
    const auto onClose = [&window](const sf::Event::Closed&) {
        window.close();
    };

    while (window.isOpen())
    {
      window.handleEvents(onClose);

      float speed = 5.f;

      // Move box with wasd
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W)) A.pos.y -= speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S)) A.pos.y += speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) A.pos.x -= speed;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) A.pos.x += speed;

        // Compare objects
        Manifold m = { &A, &B };
        bool collided = AABBvsAABB(&m);

        // Set up boxes
        boxA.setOutlineColor(sf::Color::Green);
        boxB.setOutlineColor(collided ? sf::Color::Red : sf::Color::Green); // Detect collisions
        boxA.setFillColor(sf::Color::Transparent);
        boxB.setFillColor(sf::Color::Transparent);
        boxA.setPosition(toSF(A.pos));
        boxB.setPosition(toSF(B.pos));

        // Set up window
        window.clear();
        window.draw(boxA);
        window.draw(boxB);

        window.display();
    }

    return 0;
}