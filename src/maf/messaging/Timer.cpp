#include <maf/messaging/Timer.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>
#include "TimerManager.h"
#include <cassert>

namespace maf { using util::IDManager; using logging::Logger;
namespace messaging {


Timer::Timer(bool cyclic) : _id(TimerManager::invalidJobID()), _cyclic(cyclic)
{
}

Timer::~Timer()
{
    stop();
}

void Timer::start(Timer::Duration milliseconds, TimeOutCallback callback)
{
    if(!callback)
    {
        Logger::error("[Timer]: Please specify not null callback");
    }
    else if((_myMgr = Component::getTimerManager()))
    {
        if(running())
        {
            Logger::info("Timer is still running, then stop!");
            stop();
        }
        auto componentRef = Component::getActiveWeakPtr();
        auto onTimeout = [componentRef, callback = std::move(callback), this]()
        {
            auto component = componentRef.lock();
            if(component)
            {
                component->postMessage<TimeoutMessage>(_id, std::move(callback));
            }
            else
            {
                if(_cyclic && _myMgr)
                {
                    _myMgr->stop(_id);
                }
                _id = TimerManager::invalidJobID();
            }
            if(!_cyclic)
            {
                _id = TimerManager::invalidJobID(); //mark that timer is not running anymore
            }
        };

        _id = _myMgr->start(milliseconds, onTimeout, _cyclic);
        Logger::info("Start new timer with id = " ,  _id);
    }
}

void Timer::restart()
{
    if(_myMgr)
    {
        _myMgr->restart(_id);
    }
}

void Timer::stop()
{
    if(_myMgr)
    {
        _myMgr->stop(_id);
    }
}

bool Timer::running()
{
    return _myMgr && _myMgr->isRunning(_id);
}

void Timer::setCyclic(bool cyclic)
{
	if (cyclic != _cyclic)
	{
		_cyclic = cyclic;
        if(_myMgr)
        {
            _myMgr->setCyclic(_id, cyclic);
        }
	}
}


}
}
