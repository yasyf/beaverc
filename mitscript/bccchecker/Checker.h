#include "../bccompiler/Types.h"
#include "../bccompiler/Instructions.h"

using namespace std;

namespace BC {
  class Checker {
    shared_ptr<Function> _main_function;
  private:
    void assertSize(int size, int correct_size) {
      if (size != correct_size) {
        cout << "Stack is not the correct size - is " << size << " should be " << correct_size << endl;
        exit(1);
      }
    }

    void checkFunction(shared_ptr<Function> function) {
      int current_stack_size = 0;
      for (auto instruction : function->instructions) {
        switch (instruction.operation) {
          case Operation::LoadConst:
          case Operation::LoadFunc:
          case Operation::LoadLocal:
          case Operation::LoadGlobal:
          case Operation::PushReference:
          case Operation::LoadReference:
          case Operation::AllocRecord:
          case Operation::Dup:
            current_stack_size++;
          break;

          case Operation::FieldLoad:
          case Operation::Neg: 
          case Operation::Not:
          case Operation::Goto:
          case Operation::GarbageCollect:
          case Operation::Label:
          case Operation::Swap:
          break;


          case Operation::If:
            assertSize(current_stack_size, 1);
          case Operation::StoreLocal:
          case Operation::StoreGlobal:
          case Operation::StoreReference:
          case Operation::Add:
          case Operation::Gt:
          case Operation::Geq:
          case Operation::Sub:
          case Operation::Mul:
          case Operation::Div:
          case Operation::Eq:
          case Operation::And:
          case Operation::Or:
          case Operation::Pop:
          case Operation::IndexLoad:
            current_stack_size--;
          break;

          case Operation::FieldStore:
            current_stack_size -= 2;
          break;

          case Operation::IndexStore:
            current_stack_size -= 3;
          break;

          case Operation::AllocClosure:
          case Operation::Call:
            current_stack_size -= instruction.operand0.value();
          break;

          case Operation::Return:
            assertSize(current_stack_size, 1);
            for (auto f : function->functions_) {
              checkFunction(f);
            }
            return;
          break;

          default:
            cout << "Unexpected opcode" << endl;
            exit(1);
          break;
        }
      }
      assertSize(current_stack_size, 1);
      for (auto f : function->functions_) {
        checkFunction(f);
      }
    }
  public:
    Checker(shared_ptr<Function> function) : _main_function(function) {}
    void check() {
      checkFunction(_main_function);
    }
  };
}