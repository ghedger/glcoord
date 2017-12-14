#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  TURNLEFT,
  TURNRIGHT
};

// Default camera values
const float YAW        = -90.0f;
const float PITCH      =  0.0f;
const float SPEED      =  2.5f;
const float SENSITIVTY =  0.1f;
const float ZOOM       =  45.0f;


// An abstract camera class that processes input and calculates the
// corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
  public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 PositionPrev;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Eular Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    bool m_pitchAdjust = 0;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
  {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    setPitchAdjust( false );
    updateCameraVectors();
  }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
  {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    setPitchAdjust( false );
    updateCameraVectors();
  }
    glm::vec3 getPosition()
    {
      return Position;
    }

    void setZoom( float zoom )
    {
      Zoom = zoom;
    }

    void setPitchAdjust( bool pa )
    {
      m_pitchAdjust = pa;
    }

    bool getPitchAdjust()
    {
      return m_pitchAdjust;
    }

    void setPosition( glm::vec3 pos )
    {
      Position = pos;
      //updateCameraVectors();
    }

    float getYaw()
    {
      return Yaw;
    }

    void setYaw( float yaw )
    {
      Yaw = yaw;
    }

    float getPitch()
    {
      return Pitch;
    }

    void setPitch( float pitch )
    {
      Pitch = pitch;
    }

    void saveOldPosition()
    {
      PositionPrev = Position;
    }


    glm::mat4 GetLookAtMatrix( glm::vec3 lookAt )
    {
      return glm::lookAt(Position, Position + lookAt, Up );
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
      return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
      float velocity = MovementSpeed * deltaTime;
      switch( direction )
      {
        case FORWARD:
          Position += Front * velocity;
          break;
        case BACKWARD:
          Position -= Front * velocity;
          break;
        case TURNLEFT:
          Yaw -= 45.0 * deltaTime;
					Pitch *= 0.95;
          break;
        case TURNRIGHT:
          Yaw += 45.0 * deltaTime;
					Pitch *= 0.95;
          break;
        case LEFT:
          Position -= Right * velocity;
          break;
        case RIGHT:
          Position += Right * velocity;
          break;
        default:
          break;
      }
      //updateCameraVectors();
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
      xoffset *= MouseSensitivity;
      yoffset *= MouseSensitivity;

      Yaw   += xoffset;
      Pitch += yoffset;

      // Make sure that when pitch is out of bounds, screen doesn't get flipped
      if (constrainPitch)
      {
        if (Pitch > 89.0f)
          Pitch = 89.0f;
        if (Pitch < -89.0f)
          Pitch = -89.0f;
      }

      // Update Front, Right and Up Vectors using the updated Eular angles
      //updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
      if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset;
      if (Zoom <= 1.0f)
        Zoom = 1.0f;
      if (Zoom >= 45.0f)
        Zoom = 45.0f;
    }

    void UpdateVectors()
    {
      updateCameraVectors();
    }

  private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
      if( m_pitchAdjust ) {
        float CamPitch = Pitch;
        // Calculate Phi (Quick hack -- all this needs clean up)
        if( Position != PositionPrev ) {
          glm::vec3 delta = Position - PositionPrev;

          double xzDelta = sqrt(
              ( double ) ( delta.x * delta.x ) +
              ( double ) ( delta.z * delta.z )
              );
          double totalDelta = sqrt(
              ( double ) ( delta.x * delta.x ) +
              ( double ) ( delta.y * delta.y ) +
              ( double ) ( delta.z * delta.z )
              );
          if( 1 ) {
            //Pitch = asin( xzDelta / delta.y ) * 180 / M_PI;
            //CamPitch = atan2( ( double )xzDelta, (double )delta.y ) * 10.0;
            CamPitch = atan2( ( double )delta.y, (double )xzDelta ) * 180.0 / M_PI;
            std::cout << delta.y << " " << xzDelta << " " << CamPitch << std::endl;
            //Pitch += (CamPitch - Pitch) / 10;
            if( CamPitch < 90.0 ) {
              if( Pitch < CamPitch ) {
                Pitch += 0.5;
              }
              if( Pitch > CamPitch ) {
                Pitch -= 0.5;
              }
              if( Pitch < -45.0 ) {
                Pitch = -45.0;
              }
              if( Pitch > 45.0 ) {
                Pitch = 45.0;
              }

            }
            //std::cout << CamPitch << " " << xzDelta << " " << delta.y << std::endl;
          }
        }
      }
      // Calculate the new Front vector
      glm::vec3 front;
      front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
      front.y = sin(glm::radians(Pitch));
      front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
      Front = glm::normalize(front);
      // Also re-calculate the Right and Up vector
      Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
      Up    = glm::normalize(glm::cross(Right, Front));
    }
};
