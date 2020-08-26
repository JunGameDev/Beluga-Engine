#pragma once
#include <memory>
#include <string>

namespace Bel
{
    enum class SoundType
    {
        kType_mp3,
        kType_Wav,
        kType_Ogg,
        kCount,
        kUnknown
    };

    struct AudioClip
    {
        bool        m_loop;
        bool        m_mute;
        bool        m_playOnAwake;
        bool        m_padding;
        int         m_volume;
        std::string m_path;
    };

    class IAudio
    {    
    public:
        virtual ~IAudio() {}

        virtual bool Initialize() = 0;
        virtual bool PlayMusic(const char* pFileName, int volume = 128) = 0;
        virtual bool PlaySoundEffect(const char* pFileName, int volume = 128) = 0;

        virtual bool ChangeMusic(const char* pFileName, int inOutMs) = 0;

        virtual void ResumeMusic() = 0;
        virtual void PauseMusic() = 0;

        virtual void DecVolume(int level = 5) = 0;
        virtual void IncVolume(int level = 5) = 0;
        virtual void Mute() = 0;

        virtual bool FadeInMusic(int inOutMs) = 0;
        virtual bool FadeOutMusic(int inOutMs) = 0;

        static std::unique_ptr<IAudio> Create();
    };
}