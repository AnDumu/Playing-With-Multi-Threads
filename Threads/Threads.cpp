// Threads.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <regex>
#include <fstream>
#include <future>

DWORD WINAPI MainThread(LPVOID lpParam);
class cOMP;

cOMP* OMP;

#define WINDOWS 0

std::mutex mutex;

struct tEvent
{
    int index;
    std::string web;
    std::string type;
};

int counters;
int counters2;
int counter_class;

bool isValidWEB(std::string & web)
{
    // Regex to check valid web
    //_^(?:(?:https?|ftp)://)(?:\S+(?::\S*)?@)?(?:(?!10(?:\.\d{1,3}){3})(?!127(?:\.\d{1,3}){3})(?!169\.254(?:\.\d{1,3}){2})(?!192\.168(?:\.\d{1,3}){2})(?!172\.(?:1[6-9]|2\d|3[0-1])(?:\.\d{1,3}){2})(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:(?:[a-z\x{00a1}-\x{ffff}0-9]+-?)*[a-z\x{00a1}-\x{ffff}0-9]+)(?:\.(?:[a-z\x{00a1}-\x{ffff}0-9]+-?)*[a-z\x{00a1}-\x{ffff}0-9]+)*(?:\.(?:[a-z\x{00a1}-\x{ffff}]{2,})))(?::\d{2,5})?(?:/[^\s]*)?$_iuS
    const std::regex pattern("web: ((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

    if (web.empty()) return false;

    return regex_match(web, pattern);
}

std::string M_Lcase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    return str;
}

bool isNumber(const std::string & str)
{
    for (char const& c : str)
    {
        if (std::isdigit(c) == 0) return false;
    }

    return true;
}

class cPokemon
{
public:

    cPokemon(std::string name, std::string web, DWORD magicNumber, std::string type)
    {
        this->name_ = name;
        this->web_ = web;
        this->magicNumber_ = magicNumber;
        this->type_ = type;

        this->tick_ = 0;
        this->votes_ = 0;
        this->processing_ = FALSE;
    }

    ~cPokemon()
    {

    }

    std::string getName()
    {
        return this->name_;
    }

    std::string getweb()
    {
        return this->web_;
    }

    DWORD getMagicNumber()
    {
        return this->magicNumber_;
    }

    std::string getType()
    {
        return this->type_;
    }

    /*
    * 
    * 7 Threads:
    *           With Lock + sleep 23 + tick 45:     137015 ms
    *           With Lock + tick 45:                68828 ms
    *           With Lock:                          24938 ms
    *           With Lock + getProcessing_FALSE:    14422 ms
    * 
    *           WithOut Lock + sleep 23 + tick 45:  27969 ms
    *           WithOut Lock + tick 45:             68844 ms
    *           WithOut Lock:                       24907 ms
    *           WithOut Lock + getProcessing_FALSE: 13656 ms
    * 
    *  5 Threads:
    *           With Lock + sleep 23 + tick 45:     136875 ms
    *           With Lock + tick 45:                68938 ms
    *           With Lock:                          25281 ms
    *           With Lock + getProcessing_FALSE:    14094 ms
    * 
    *           WithOut Lock + sleep 23 + tick 45:  37079 ms
    *           WithOut Lock + tick 45:             68828 ms
    *           WithOut Lock:                       25375 ms
    *           WithOut Lock + getProcessing_FALSE: 14859 ms
    * 
    * 1 Thread:
    *           With Lock + sleep 23 + tick 45:     138297 ms
    *           With Lock + tick 45:                69219 ms
    *           With Lock:                          25266 ms
    *           With Lock + getProcessing_FALSE:    11969 ms
    * 
    *           WithOut Lock + sleep 23 + tick 45:  138312 ms
    *           WithOut Lock + tick 45:             68860 ms
    *           WithOut Lock:                       25187 ms
    *           WithOut Lock + getProcessing_FALSE: 12547 ms
    * 
    */
    void Perform(std::string web, std::string type)
    {
        //std::unique_lock<std::mutex> lock(mutex);

        counter_class++;
        this->votes_++;

        this->type_ = type;

        //std::this_thread::sleep_for(std::chrono::milliseconds(23));

        this->tick_ = GetTickCount64();
        this->processing_ = FALSE;
    }

    void Perform_Windows()
    {
        //std::unique_lock<std::mutex> lock(mutex);

        counter_class++;
        this->votes_++;

        this->type_ = this->event_.type;
        this->event_ = {};

        //Sleep(23);

        this->tick_ = GetTickCount64();
        this->processing_ = FALSE;
    }

    BOOL getProcessing()
    {
        //return FALSE;
        return (this->processing_ || GetTickCount64() - this->tick_ <= 45);
        //return (this->processing_ || GetTickCount64() - this->tick_ <= 0);
    }

    DWORD getVotes()
    {
        return this->votes_;
    }

    void setEvent(tEvent event)
    {
        this->processing_ = TRUE;
        this->event_ = event;
    }

    std::string & getEventWeb()
    {
        return this->event_.web;
    }

    std::string & getEventType()
    {
        return this->event_.type;
    }

private:
    std::string name_;
    std::string web_;
    DWORD magicNumber_;
    std::string type_;

    ULONGLONG tick_;
    DWORD votes_;
    BOOL processing_;

    tEvent event_;
};

DWORD WINAPI PokemonProcessThread(LPVOID lpParam)
{
    counters2++;
    ((cPokemon*)lpParam)->Perform_Windows();

    return 0;
}

class cOMP //Organización Mundial de los Pokemons, pero ésta no es como la OMS que da información falsa
{
public:
    cOMP(BYTE max_threads, BOOL windows)
    {
        int threads = 0;

        this->windows_ = windows;
        this->running_ = TRUE;
        this->ending_ = FALSE;
        this->erasing_ = FALSE;

        if (this->windows_)
        {
            this->mainThreadWindows_ = CreateThread(NULL, 0, &MainThread, NULL, 0, &this->threadId_);

            if (this->mainThreadWindows_ == NULL)
            {
                std::cout << "Error creating main thread" << std::endl;
                std::cin.get();
                ExitProcess(3);
            }
        }
        else
        {
            this->mainThread_ = std::thread(&cOMP::processor, this);
            this->mainThread_.detach();
        }

        switch (max_threads)
        {
            case 12:
                threads = 4;

                break;
            case 14:
                threads = 5;

                break;
            case 16:
                threads = 7;

                break;
            case 18:
                threads = 9;

                break;
            default:
                threads = 2;
                break;
        }

        if (max_threads > 18) threads = 12;

        //testing stuff
        //threads = 5;

        if (this->windows_)
        {
            this->thsWin_.resize(threads);
            this->thsWinId_.resize(threads);
        }
        else
        {
            this->ths_.resize(threads);
        }
    }

    ~cOMP()
    {
        for (int i = 0; i < this->pokemons_.size(); i++)
        {
            delete this->pokemons_[i];
            this->pokemons_[i] = NULL;
        }
    }

    void processPokemon(std::vector<std::string> info)
    {
        std::string pokemon;
        std::string web;
        std::string type;
        DWORD magicNumber;
        int index = -1;
        BOOL err = FALSE;

        try
        {
            magicNumber = std::stoul(info[2].c_str(), NULL, 0);
            web = info[1];
            pokemon = info[0];
            type = info[3];

            for (int i = 0; i < this->pokemons_.size(); i++)
            {
                if (M_Lcase(this->pokemons_[i]->getName()) == M_Lcase(pokemon))
                {
                    if (this->pokemons_[i]->getMagicNumber() != magicNumber) return;

                    index = i;
                    break;
                }
            }
        }
        catch (std::exception& e)
        {
            err = TRUE;
        }

        if (err) return;

        if (index == -1)
        {
            index = this->pokemons_.size();
            this->pokemons_.push_back(new cPokemon(pokemon, web, magicNumber, type));
        }

        tEvent event;

        event.index = index;
        event.web = web;
        event.type = type;

        while (this->erasing_)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(333));
        }

        //std::cout << "adding event" << std::endl;

        counters++;

        this->pendingEvents_.push_back(event);
    }

    DWORD getPokemonsVotes()
    {
        if (this->windows_)
        {
            return this->getPokemonsvotes_windows();
        }
        else
        {
            return this->getPokemonsvotes_other();
        }
    }

    DWORD getPokemonsvotes_windows()
    {
        DWORD votes = 0;
        BOOL flag = FALSE;

        std::cout << "getPokemonsvotes_Windows" << std::endl;

        while (TRUE)
        {
            if (this->processingEvents_.size() == 0 && this->failedEvents_.size() == 0 && this->pendingEvents_.size() == 0)
            {
                for (size_t i = 0; i < this->thsWin_.size(); i++)
                {
                    if (this->isThreadAlive(this->thsWin_[i])) break;

                    if (i == this->thsWin_.size() - 1)
                    {
                        flag = TRUE;
                    }
                }
            }

            if (flag) break;

            std::this_thread::sleep_for(std::chrono::nanoseconds(333));
        }

        while (!this->ending_)
        {
            this->running_ = FALSE;
            std::this_thread::sleep_for(std::chrono::nanoseconds(333));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (size_t i = 0; i < this->pokemons_.size(); i++)
        {
            votes += this->pokemons_[i]->getVotes();
        }

        return votes;
    }

    DWORD getPokemonsvotes_other()
    {
        DWORD votes = 0;
        BOOL flag = FALSE;

        std::cout << "getPokemonsvotes_other" << std::endl;

        while (TRUE)
        {
            if (this->processingEvents_.size() == 0 && this->failedEvents_.size() == 0 && this->pendingEvents_.size() == 0)
            {
                for (size_t i = 0; i < this->ths_.size(); i++)
                {
                    if (!this->future_is_ready(this->ths_[i])) break;

                    if (i == this->ths_.size() - 1)
                    {
                        flag = TRUE;
                    }
                }
            }

            if (flag) break;

            std::this_thread::sleep_for(std::chrono::nanoseconds(333));
        }

        while (!this->ending_)
        {
            this->running_ = FALSE;
            std::this_thread::sleep_for(std::chrono::nanoseconds(333));
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (size_t i = 0; i < this->pokemons_.size(); i++)
        {
            votes += this->pokemons_[i]->getVotes();
        }

        return votes;
    }

    BOOL getEraseing()
    {
        return this->erasing_;
    }

    BOOL isThreadAlive(HANDLE th)
    {
        DWORD ExitCode;

        GetExitCodeThread(th, &ExitCode);

        return (ExitCode == STILL_ACTIVE);
    }

    void processor_windows()
    {
        int thread_index = -1;
        std::string title;

        while (this->thsWin_.size() == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        while (this->running_ || this->processingEvents_.size() > 0)
        {
            title = "ProcessingEvents: " + std::to_string(this->processingEvents_.size()) + " - FailedEvents: " + std::to_string(this->failedEvents_.size()) + " - PendingEvents: " + std::to_string(this->pendingEvents_.size());
            SetConsoleTitleA(title.c_str());

            if (this->failedEvents_.size() > 0)
            {
                this->processingEvents_.insert(this->processingEvents_.end(), this->failedEvents_.begin(), this->failedEvents_.end());
                this->failedEvents_.clear();
            }
            
            if (this->pendingEvents_.size() > 0)
            {
                this->processingEvents_.insert(this->processingEvents_.end(), this->pendingEvents_.begin(), this->pendingEvents_.end());
                this->pendingEvents_.clear();
            }
            
            this->erasing_ = FALSE;

            for (int i = 0; i < this->processingEvents_.size(); i++)
            {
                title = "ProcessingEvents: " + std::to_string(this->processingEvents_.size()) + " - FailedEvents: " + std::to_string(this->failedEvents_.size()) + " - PendingEvents: " + std::to_string(this->pendingEvents_.size());
                SetConsoleTitleA(title.c_str());

                size_t pokemon_index = this->processingEvents_[i].index;

                if (this->pokemons_[pokemon_index]->getProcessing())
                {
                    this->failedEvents_.push_back(this->processingEvents_[i]);
                    continue;
                }

                while (TRUE)
                {
                    for (size_t i2 = 0; i2 < this->thsWin_.size(); i2++)
                    {
                        if (!this->isThreadAlive(this->thsWin_[i2]))
                        {
                            if (this->thsWin_[i2] != NULL)
                            {
                                CloseHandle(this->thsWin_[i2]);
                            }

                            thread_index = i2;
                            break;
                        }
                    }

                    if (thread_index != -1) break;
                }

                this->pokemons_[pokemon_index]->setEvent(this->processingEvents_[i]);

                std::cout << std::dec << "Using thread index " << thread_index << std::endl;
                std::cout << "\t" << "Name: " << this->pokemons_[pokemon_index]->getName().c_str() << std::endl;
                std::cout << "\t" << "Web: " << this->processingEvents_[i].web.c_str() << std::endl;
                std::cout << "\t" << "Type: " << this->processingEvents_[i].type.c_str() << std::endl;
                std::cout << "\t" << std::hex << (DWORD)&this->pokemons_[pokemon_index] << std::dec << std::endl;

                this->thsWin_[thread_index] = CreateThread(NULL, 0, &PokemonProcessThread, this->pokemons_[pokemon_index], 0, &this->thsWinId_[thread_index]);

                thread_index = -1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            this->erasing_ = TRUE;
            this->processingEvents_.clear();
        }

        this->ending_ = TRUE;
    }

    int getThreads()
    {
        if (this->windows_)
        {
            return this->thsWin_.size();
        }
        else
        {
            return this->ths_.size();
        }        
    }

protected:
    template<typename T>
    BOOL future_is_ready(std::future<T>& t)
    {
        BOOL hRet;

        try
        {
            hRet = (t.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
        }
        catch (std::exception& e)
        {
            hRet = TRUE;
        }

        return hRet;
    }

    void processor()
    {
        int thread_index = -1;
        std::string title;

        while (this->ths_.size() == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        while (this->running_ || this->processingEvents_.size() > 0)
        {
            title = "ProcessingEvents: " + std::to_string(this->processingEvents_.size()) + " - FailedEvents: " + std::to_string(this->failedEvents_.size()) + " - PendingEvents: " + std::to_string(this->pendingEvents_.size());
            SetConsoleTitleA(title.c_str());

            if (this->failedEvents_.size() > 0)
            {
                this->processingEvents_.insert(this->processingEvents_.end(), this->failedEvents_.begin(), this->failedEvents_.end());
                this->failedEvents_.clear();
            }

            if (this->pendingEvents_.size() > 0)
            {
                this->processingEvents_.insert(this->processingEvents_.end(), this->pendingEvents_.begin(), this->pendingEvents_.end());
                this->pendingEvents_.clear();
            }

            this->erasing_ = FALSE;

            for (int i = 0; i < this->processingEvents_.size(); i++)
            {
                title = "ProcessingEvents: " + std::to_string(this->processingEvents_.size()) + " - FailedEvents: " + std::to_string(this->failedEvents_.size()) + " - PendingEvents: " + std::to_string(this->pendingEvents_.size());
                SetConsoleTitleA(title.c_str());

                size_t pokemon_index = this->processingEvents_[i].index;

                if (this->pokemons_[pokemon_index]->getProcessing())
                {
                    this->failedEvents_.push_back(this->processingEvents_[i]);
                    continue;
                }

                while (TRUE)
                {
                    for (size_t i2 = 0; i2 < this->ths_.size(); i2++)
                    {
                        if (this->future_is_ready(this->ths_[i2]))
                        {
                            thread_index = i2;
                            break;
                        }
                    }

                    if (thread_index != -1) break;
                }

                std::cout << std::dec << "Using thread index " << thread_index << std::endl;
                std::cout << "\t" << "Name: " << this->pokemons_[pokemon_index]->getName().c_str() << std::endl;
                std::cout << "\t" << "Web: " << this->processingEvents_[i].web.c_str() << std::endl;
                std::cout << "\t" << "Type: " << this->processingEvents_[i].type.c_str() << std::endl;
                std::cout << "\t" << std::hex << (DWORD)&this->pokemons_[pokemon_index] << std::dec << std::endl;

                //thread
                this->ths_[thread_index] = std::async(std::launch::async, &cPokemon::Perform, this->pokemons_[pokemon_index], this->processingEvents_[i].web, this->processingEvents_[i].type);

                /*this->ths_[thread_index] = std::thread(&cPokemon::Perform, &this->pokemons_[pokemon_index], this->events_[i].web, this->events_[i].type);
                if (this->ths_[thread_index].joinable())
                {
                    //this->ths_[thread_index].join(); //blocking shit (useless)
                    this->ths_[thread_index].detach(); //dunno how to check thread status without using Windows's APIs
                }*/

                thread_index = -1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            this->erasing_ = TRUE;
            this->processingEvents_.clear();
        }

        this->ending_ = TRUE;
    }

private:
    std::thread mainThread_;
    HANDLE mainThreadWindows_;
    DWORD threadId_;

    //std::vector<std::thread> ths_; //dunno how to check if the thread is alive... awful...
    std::vector<std::future<void>> ths_; //future? omg C++, what did they do to you? Well, it is bad but not dat bad
    std::vector<HANDLE> thsWin_;
    std::vector<DWORD> thsWinId_;

    std::vector<cPokemon*> pokemons_;

    std::vector<tEvent> processingEvents_;
    std::vector<tEvent> failedEvents_;
    std::vector<tEvent> pendingEvents_;

    BOOL windows_;
    BOOL running_;
    BOOL ending_;
    BOOL erasing_;
};

DWORD WINAPI MainThread(LPVOID lpParam)
{
    OMP->processor_windows();

    return 0;
}

class cFilter
{
public:
    cFilter()
    {
        this->step_ = 0;
    }

    BOOL process(std::string & incoming)
    {
        switch (this->step_)
        {
            case 4:
            case 0:
                this->step_ = 0;
                this->tags_.clear();

                if (incoming.rfind("pokemon: ", 0) != std::string::npos)
                {
                    this->tags_.push_back(incoming.substr(9));
                    this->step_++;
                }

                break;
            case 1:
                if (isValidWEB(incoming))
                {
                    this->tags_.push_back(incoming.substr(5));
                    this->step_++;
                    break;
                }

                this->step_ = 0;
                this->tags_.clear();

                break;
            case 2:
                if (isNumber(incoming.substr(13)))
                {
                    this->tags_.push_back(incoming.substr(13));
                    this->step_++;
                    break;
                }

                this->step_ = 0;
                this->tags_.clear();

                break;
            case 3:
                if (incoming.rfind("type: ", 0) != std::string::npos)
                {
                    this->tags_.push_back(incoming.substr(6));
                    this->step_++;
                    break;
                }

                this->step_ = 0;
                this->tags_.clear();

                break;
        }

        return (this->step_ == 4);
    }

    std::vector<std::string> & retrieveValues()
    {
        return this->tags_;
    }

private:
    int step_;
    std::vector<std::string> tags_;
};

int main()
{
    std::string device{};
    std::string web{};
    std::string timestampStr{};
    unsigned long long timestamp = 0;
    bool passed = false;

    std::cout << "max_cpus: " << std::thread::hardware_concurrency() << std::endl;

    std::string fileName = R"(a.txt)";

    std::ifstream infile(fileName.c_str());

    counters = 0;
    counters2 = 0;
    counter_class = 0;

    cFilter Filter;
    OMP = new cOMP(std::thread::hardware_concurrency(), WINDOWS);

    ULONGLONG B, E;

    B = GetTickCount64();

    for (std::string line; std::getline(infile, line);)
    {
        if (Filter.process(line))
        {
            OMP->processPokemon(Filter.retrieveValues());
        }
    }

    std::cout << "Votes: " << OMP->getPokemonsVotes() << std::endl;
    std::cout << "processPokemon Counters: " << counters << std::endl;
    std::cout << "PokemonProcessThread (Windows) Counters2: " << counters2 << std::endl;
    std::cout << "counter_class: " << counter_class << std::endl;
    std::cout << "Threads: " << OMP->getThreads() << std::endl;

    delete OMP;
    OMP = NULL;

    E = GetTickCount64();

    std::cout << "Time: " << E - B << std::endl;

    std::cin.get();

    return 0;
}