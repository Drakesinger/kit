#ifndef KIT_SKELETON_HPP
#define KIT_SKELETON_HPP

#include "Kit/Export.hpp"
#include "Kit/Types.hpp"
#include "Kit/Transformable.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <map>

namespace kit 
{

  class KITAPI Skeleton 
  {
    public:
      
      struct AnimationChannel
      {
        glm::vec3 getTranslationAt(float frametime); //< Time in frames
        glm::quat getRotationAt(float frametime); //< Time in frames
        glm::vec3 getScaleAt(float frametime); //< Time in frames
        
        glm::mat4 getTransformMatrix(float frametime); //< Gets a full transformation matrix at the current time for the current bone
        
        std::map<float, glm::vec3> m_translationKeys; //< float-key is the timeframe (actual frames), value is translation
        std::map<float, glm::quat> m_rotationKeys;    //< float-key is the timeframe (actual frames), value is rotation
        std::map<float, glm::vec3> m_scaleKeys;       //< float-key is the timeframe (actual frames), value is scale
      };
      
      struct Animation
      {
        typedef std::shared_ptr<kit::Skeleton::Animation> Ptr;

        glm::mat4 getBoneTransform(uint32_t boneId, float frame);

        std::string m_name;
        std::map<uint32_t, kit::Skeleton::AnimationChannel>  m_channels; //< Key is bone ID
        float               m_framesPerSecond;
        float               m_frameDuration;
      };
        
      struct Bone
      {
        typedef std::shared_ptr<Bone> Ptr;

        glm::quat getCurrentRotation();
        glm::vec3 getCurrentPosition();

        uint32_t m_id;
        std::string m_name;

        uint32_t m_parentId;
        kit::Skeleton::Bone::Ptr m_parent;

        std::vector<kit::Skeleton::Bone::Ptr> m_children;

        glm::mat4 m_localTransform;
        glm::mat4 m_globalTransform;
      };
      
      typedef std::shared_ptr<kit::Skeleton> Ptr;
      typedef std::pair<glm::mat4, Bone::Ptr> WorkPair;

      static kit::Skeleton::Ptr load(const std::string& filename);
      bool save(const std::string& filename);
      
      void update(const double & ms);
      void updateSkin();

      void setAnimation(const std::string& name);

      void play(bool loop = true);
      void pause();
      void stop();

      ~Skeleton();
  
      std::vector<glm::mat4> getSkin();

      kit::Skeleton::Bone::Ptr getBone(const std::string& name);
      kit::Skeleton::Bone::Ptr getBone(uint32_t id);
      
      kit::Skeleton::Animation::Ptr getAnimation(const std::string& animationname);

      Skeleton();
    private:

      std::vector<kit::Skeleton::Bone::Ptr>             m_rootBones;
      std::map<std::string, kit::Skeleton::Bone::Ptr>   m_boneIndexName;
      std::vector<kit::Skeleton::Bone::Ptr>             m_boneIndexId;
      std::vector<glm::mat4>                            m_inverseBindPose;
      std::vector<glm::mat4>                            m_skin;

      glm::mat4 m_globalInverseTransform;

      // Animation
      kit::Skeleton::Animation::Ptr                        m_currentAnimation;
      std::map<std::string, kit::Skeleton::Animation::Ptr> m_animations;

      
      double m_currentTime; //< Time in milliseconds
      float m_currentFrame; //<  Current animation frame
      bool m_isPlaying;
      bool m_isLooping;
  };
  
}

#endif
