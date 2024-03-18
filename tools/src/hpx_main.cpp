#include <solarsim/types.hpp>
#include <solarsim/namespaces.hpp>

#include <hpx/execution.hpp>
#include <hpx/iostream.hpp>
// Including 'hpx/hpx_main.hpp' instead of the usual 'hpx/hpx_init.hpp' enables
// to use the plain C-main below as the direct main HPX entry point.
#include <hpx/hpx_main.hpp>

#include <cassert>

SOLARSIM_NS_BEGIN

void test_hpx_execution_ops()
{
  const auto [v] = (ex::just(1010) | tt::sync_wait()).value();
  assert(v == 1010);
}

SOLARSIM_NS_END

int main()
{
  solarsim::test_hpx_execution_ops();
  return 0;
}
