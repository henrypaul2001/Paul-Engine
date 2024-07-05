#pragma once
#include "StateTransition.h"
namespace Engine {
    template <class T, class U>
    class GenericStateTransition : public StateTransition
    {
    public:
        typedef bool (*GenericTransitionFunc)(T, U);

        GenericStateTransition(GenericTransitionFunc function, T data, U otherData, State* source, State* destination) : StateTransition(source, destination), dataA(data), dataB(otherData) {
            this->function = function;
        }
        ~GenericStateTransition() {}

        virtual bool Condition() const override {
            if (function) {
                return function(dataA, dataB);
            }
            return false;
        }

        // Generic condition functions
        static bool GreaterThanCondition(T dataA, U dataB) {
            return dataA > dataB;
        }
        static bool LessThanCondition(T dataA, U dataB) {
            return dataA < dataB;
        }
        static bool EqualToCondition(T dataA, U dataB) {
            return dataA == dataB;
        }
        static bool NotEqualToCondition(T dataA, U dataB) {
            return dataA != dataB;
        }

    protected:
        GenericTransitionFunc function;
        T dataA;
        U dataB;
    };
}