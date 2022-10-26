#ifndef Scope
#define Scope

#include "ASM.hpp"
#include "CodeGenerator/CodeGenerator.hpp"
#include <vector>

namespace gen {

namespace scope {

class ScopeManager {

private:

  struct plead {
    int SScopeSize;
    bool pleading;
  };
  ScopeManager();
  ~ScopeManager() = default;
  static ScopeManager *instance;

  // Stack
  std::vector<gen::Symbol> stack;

  // Global Scope
  std::vector<gen::Symbol> globalStack;

  std::vector<plead> pleading;

  // hold the current memory location relative to the rbp
  int stackPos;

  int maxStackPos;

  // scopeStack holds the number of symbols in the current scope
  std::vector<int> scopeStack;
  int SStackSize = 0;

  // int scopeStackPos;

public:
  // Singleton Access
  static ScopeManager *getInstance() {
    if (!ScopeManager::instance)
      ScopeManager::instance = new ScopeManager;
    return ScopeManager::instance;
  };

  // cannot be copied
  ScopeManager(ScopeManager const &) = delete;

  // cannot be assigned
  void operator=(ScopeManager const &) = delete;

  // Assign a new symbol and return the byteMod
  int assign(std::string symbol, ast::Type type, bool mask, bool mut = true);

  // Get a symbol
  gen::Symbol *get(std::string symbol);

  // push a new scope
  void pushScope(bool func);

  // pop a scope
  void popScope(gen::CodeGenerator *callback, asmc::File &OutputFile,
                bool fPop = false);

  void softPop(gen::CodeGenerator *callback, asmc::File &OutputFile);

  // Get stack alignment value
  int getStackAlignment();

  // reset everything
  void reset();
};

} // namespace scope

} // namespace gen
#endif