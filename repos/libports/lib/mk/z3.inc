#
# Z3 solver library
#

Z3_PORT_DIR := $(call select_from_ports,z3)
Z3_DIR      := $(Z3_PORT_DIR)/src/lib/z3

D_OPTS   = _NO_OMP_ _MP_GMP
D_OPTS  := $(addprefix -D,$(D_OPTS))
CC_OPT  += $(D_OPTS)

INC_DIR += $(REP_DIR)/src/lib/z3/include \
           $(Z3_PORT_DIR)/include/z3 \
           $(Z3_DIR)/src/util \
           $(Z3_DIR)/src/math/polynomial \
           $(Z3_DIR)/src/sat \
           $(Z3_DIR)/src/nlsat \
           $(Z3_DIR)/src/math/hilbert \
           $(Z3_DIR)/src/math/simplex \
           $(Z3_DIR)/src/math/automata \
           $(Z3_DIR)/src/math/interval \
           $(Z3_DIR)/src/math/realclosure \
           $(Z3_DIR)/src/math/subpaving \
           $(Z3_DIR)/src/ast \
           $(Z3_DIR)/src/ast/rewriter \
           $(Z3_DIR)/src/ast/normal_forms \
           $(Z3_DIR)/src/model \
           $(Z3_DIR)/src/tactic \
           $(Z3_DIR)/src/ast/substitution \
           $(Z3_DIR)/src/parsers/util \
           $(Z3_DIR)/src/math/grobner \
           $(Z3_DIR)/src/math/euclid \
           $(Z3_DIR)/src/tactic/core \
           $(Z3_DIR)/src/sat/tactic \
           $(Z3_DIR)/src/tactic/arith \
           $(Z3_DIR)/src/nlsat/tactic \
           $(Z3_DIR)/src/math/subpaving/tactic \
           $(Z3_DIR)/src/tactic/aig \
           $(Z3_DIR)/src/solver \
           $(Z3_DIR)/src/interp \
           $(Z3_DIR)/src/cmd_context \
           $(Z3_DIR)/src/cmd_context/extra_cmds \
           $(Z3_DIR)/src/parsers/smt2 \
           $(Z3_DIR)/src/ast/proof_checker \
           $(Z3_DIR)/src/ast/simplifier \
           $(Z3_DIR)/src/ast/fpa \
           $(Z3_DIR)/src/ast/macros \
           $(Z3_DIR)/src/ast/pattern \
           $(Z3_DIR)/src/ast/rewriter/bit_blaster \
           $(Z3_DIR)/src/smt/params \
           $(Z3_DIR)/src/smt/proto_model \
           $(Z3_DIR)/src/smt \
           $(Z3_DIR)/src/tactic/bv \
           $(Z3_DIR)/src/test/fuzzing \
           $(Z3_DIR)/src/smt/tactic \
           $(Z3_DIR)/src/tactic/sls \
           $(Z3_DIR)/src/qe \
           $(Z3_DIR)/src/duality \
           $(Z3_DIR)/src/muz/base \
           $(Z3_DIR)/src/muz/dataflow \
           $(Z3_DIR)/src/muz/transforms \
           $(Z3_DIR)/src/muz/rel \
           $(Z3_DIR)/src/muz/pdr \
           $(Z3_DIR)/src/muz/clp \
           $(Z3_DIR)/src/muz/tab \
           $(Z3_DIR)/src/muz/bmc \
           $(Z3_DIR)/src/muz/ddnf \
           $(Z3_DIR)/src/muz/duality \
           $(Z3_DIR)/src/muz/fp \
           $(Z3_DIR)/src/tactic/nlsat_smt \
           $(Z3_DIR)/src/tactic/smtlogics \
           $(Z3_DIR)/src/tactic/fpa \
           $(Z3_DIR)/src/tactic/ufbv \
           $(Z3_DIR)/src/sat/sat_solver \
           $(Z3_DIR)/src/tactic/portfolio \
           $(Z3_DIR)/src/parsers/smt \
           $(Z3_DIR)/src/opt \
           $(Z3_DIR)/src/api
