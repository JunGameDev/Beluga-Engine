#include "Core/Layers/ApplicationLayer.h"
#include <unordered_map>
#include <SDL.h>
#include <SDL_mixer.h>

#include "Audio/Audio.h"

using namespace Bel;

class SDLAudio : public IAudio
{
public:
    using SoundMap = std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)>>;
private:
    bool m_initialized;
    
    // This was the key
    std::vector<char> m_musicData;
    std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)> m_pMusic;
    SoundMap m_sounds;

    int m_volume;

public:
    SDLAudio() 
        : m_initialized(false)
        , m_pMusic(nullptr, nullptr)
        , m_volume(SDL_MIX_MAXVOLUME)
    {
        
    }
    virtual ~SDLAudio()
    {
        m_sounds.clear();

        if(m_initialized)
        {
            //MIX_//m_pMusic = nullptr;
            m_pMusic.release();
            Mix_CloseAudio();
            Mix_Quit();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
        }

    }

    virtual bool Initialize() override
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO))
        {
            LOG_ERROR("Unable to initialize SDL Audio");
            LOG_ERROR(SDL_GetError());
            return false;
        }

        m_initialized = true;

        if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS, 1024))
        {
            LOG_ERROR("Unable to open SDL Mixer");
            LOG_ERROR(Mix_GetError());
            return false;
        }

        int mixFlags = MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG;
        int initialized = Mix_Init(mixFlags);

        if (initialized != mixFlags)
        {
            LOG_ERROR("Unable to initialize SDL Mixer with all formats requested");
            LOG_ERROR(Mix_GetError());
            return false;
        }

        return true;
    }

    virtual bool PlayMusic(const char* pFileName, int volume = 128) override
    {
        ApplicationLayer*   pApp    = ApplicationLayer::GetInstance();
        ResourceCache*      pCache  = nullptr;
        
        if (pApp != nullptr)
            pCache = ApplicationLayer::GetInstance()->GetGameLayer()->GetResourceCache();
        
        SDL_RWops* pRwops = nullptr;
        Mix_Music* pMusic = nullptr;

        // With resource cache
        if (pCache != nullptr)
        {
            auto pResource = pCache->GetHandle(&Resource(pFileName));
            m_musicData.clear();
            m_musicData = pResource->GetData();

            SDL_RWops* pRwops = SDL_RWFromMem(m_musicData.data(), static_cast<int>(m_musicData.size()));
            Mix_Music* pMusic = Mix_LoadMUS_RW(pRwops, 0);

            m_pMusic = std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>(pMusic, &Mix_FreeMusic);
        }
        else
        {
            m_pMusic = std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>(Mix_LoadMUS(pFileName), &Mix_FreeMusic);
        }

        if (m_pMusic == nullptr)
        {
            LOG_ERROR("Failed to load music ", false);
            LOG_ERROR(pFileName);
            LOG_ERROR(Mix_GetError());
            return false;
        }

        m_volume = volume;
        Mix_VolumeMusic(m_volume);

        if (Mix_PlayMusic(m_pMusic.get(), -1))
        {
            LOG_ERROR("Unable to play music ", false);
            LOG_ERROR(pFileName);
            LOG_ERROR(Mix_GetError());

            return false;
        }

        return true;
    }

    virtual bool PlaySoundEffect(const char* pFileName, int volume = 128) override
    {
        SoundMap::const_iterator soundIter = m_sounds.end();
        
        ApplicationLayer*  pApp     = ApplicationLayer::GetInstance();
        ResourceCache*     pCache   = nullptr;
        Mix_Chunk*         pChunk   = nullptr;

        std::string        fileName = pFileName;

        if (pApp != nullptr)
            pCache = pApp->GetGameLayer()->GetResourceCache();

        if (pCache != nullptr)
        {
            auto pResource = pCache->GetHandle(&Resource(pFileName));
            soundIter = m_sounds.find(pResource->GetName());

            if (soundIter == m_sounds.end())
            {
                SDL_RWops* pRwops = SDL_RWFromMem(pResource->GetData().data(), static_cast<int>(pResource->GetData().size()));
                pChunk = Mix_LoadWAV_RW(pRwops, 0);
                fileName = pResource->GetName();
                soundIter = m_sounds.emplace(fileName.data(), std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)>(pChunk, &Mix_FreeChunk)).first;
            }
            else
            {
                LOG_ERROR("Unable to load sound ", false);
                LOG_ERROR(fileName.data());
                LOG_ERROR(Mix_GetError());
            }
        }
        else
        {
            soundIter = m_sounds.find(pFileName);

            if (soundIter == m_sounds.end())
            {
                pChunk = Mix_LoadWAV(pFileName);
                fileName = pFileName;
            }
            else
            {
                LOG_ERROR("Unable to load sound ", false);
                LOG_ERROR(fileName.data());
                LOG_ERROR(Mix_GetError());
            }
        }

        Mix_PlayChannel(-1, soundIter->second.get(), 0);
        Mix_VolumeChunk(soundIter->second.get(), volume);

        return true;
    }

    virtual bool ChangeMusic(const char* pFileName, int inOutMs) override
    {
        FadeOutMusic(inOutMs);

        ApplicationLayer* pApp = ApplicationLayer::GetInstance();
        ResourceCache* pCache = nullptr;

        if (pApp != nullptr)
            pCache = ApplicationLayer::GetInstance()->GetGameLayer()->GetResourceCache();

        SDL_RWops* pRwops = nullptr;
        Mix_Music* pMusic = nullptr;

        // With resource cache
        if (pCache != nullptr)
        {
            auto pResource = pCache->GetHandle(&Resource(pFileName));
            m_musicData.clear();
            m_musicData = pResource->GetData();

            SDL_RWops* pRwops = SDL_RWFromMem(m_musicData.data(), static_cast<int>(m_musicData.size()));
            Mix_Music* pMusic = Mix_LoadMUS_RW(pRwops, 0);

            m_pMusic = std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>(pMusic, &Mix_FreeMusic);
        }
        else
        {
            m_pMusic = std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>(Mix_LoadMUS(pFileName), &Mix_FreeMusic);
        }
        //m_pMusic = std::unique_ptr<Mix_Music, decltype(&Mix_FreeMusic)>(Mix_LoadMUS(pFileName), &Mix_FreeMusic);

        FadeInMusic(inOutMs);
        return true;
    }

    virtual void ResumeMusic() override
    {
        Mix_ResumeMusic();
    }

    virtual void PauseMusic() override
    {
        Mix_PauseMusic();
    }

    virtual void DecVolume(int level) override
    {
        constexpr unsigned int kMinVolume = 0;

        m_volume -= level;

        if (m_volume < kMinVolume)
        {
            m_volume = kMinVolume;
        }

        Mix_VolumeMusic(m_volume);
    }

    virtual void IncVolume(int level) override
    {
        m_volume += level;

        if (m_volume >= SDL_MIX_MAXVOLUME)
        {
            m_volume = SDL_MIX_MAXVOLUME;
        }

        Mix_VolumeMusic(m_volume);
    }

    virtual void Mute() override
    {
        static bool mute = false;
        constexpr int halfVol = MIX_MAX_VOLUME / 2;
        mute = !mute;

        if (mute)
            Mix_VolumeMusic(0);
        else
            Mix_VolumeMusic(halfVol);
    }

    virtual bool FadeInMusic(int inOutMs) override
    {
        if (Mix_Playing(-1))
            return false;

        if (m_pMusic == nullptr)
        {
            LOG_ERROR("Unable to load music ", false);
            LOG_ERROR(Mix_GetError());

            return false;
        }

        if (Mix_FadeInMusic(m_pMusic.get(), -1, inOutMs) == -1)
        {
            LOG_ERROR("Mix_FadeinMusic: ", false);
            LOG_ERROR(Mix_GetError());
        }

        return true;
    }

    virtual bool FadeOutMusic(int inOutMs) override
    {
        // fade out music to finish 2 seconds from now
        while (!Mix_FadeOutMusic(2000) && Mix_PlayingMusic())
        {
        }

        return true;
    }

private:
    SoundMap::const_iterator GetMusicFromCache(const char* pFileName, ResourceCache* pCache)
    {
        auto pResource = pCache->GetHandle(&Resource(pFileName));
        if (pResource == nullptr)
        {
            LOG("FaileD to find music: ", false);
            LOG(pFileName);
            LOG(Mix_GetError());
            return m_sounds.end();
        }

        SoundMap::const_iterator soundItr = m_sounds.find(pResource->GetName());
        if (soundItr == m_sounds.end())
        {
            //Mix_Chunk* pChunk = Mix_LoadWAV(pFileName);

            SDL_RWops* pRwops = SDL_RWFromMem(pResource->GetData().data(), static_cast<int>(pResource->GetData().size()));
            Mix_Chunk* pChunk = Mix_LoadWAV_RW(pRwops, 0);

            if (pChunk != nullptr)
            {
                soundItr = m_sounds.emplace(pResource->GetName(), std::unique_ptr<Mix_Chunk, decltype(&Mix_FreeChunk)>(pChunk, &Mix_FreeChunk)).first;
            }
            else
            {
                LOG_ERROR("Unable to load sound ", false);
                LOG_ERROR(pResource->GetName().c_str());
                LOG_ERROR(Mix_GetError());

                return soundItr;
            }
        }

        return m_sounds.end();
    }
};

std::unique_ptr<IAudio> IAudio::Create()
{
    return std::make_unique<SDLAudio>();
}