# GCEM Test build

ifeq ($(CXX),)
	CXX = g++
endif

ifeq ($(GCEM_CXX_STD),)
	CXX_STD = -std=c++11
else
	CXX_STD=$(GCEM_CXX_STD)
endif

ifneq (,$(findstring clang,$(CXX)))
	OPT_FLAGS = -g -O0 -Wall -Wextra --coverage -fno-inline
else
	OPT_FLAGS = -g -O0 -Wall -Wextra --coverage -fno-inline -fno-inline-small-functions -fno-default-inline
endif

ifneq ($(TRAVIS_CLANG_BUILD),)
	OPT_FLAGS += -DTRAVIS_CLANG_CXX
endif

# source directories
SDIR = .
HEADER_DIR = $(SDIR)/../include

SOURCES_TESTS= $(wildcard $(SDIR)/*.cpp)
OBJECTS_TESTS= $(SOURCES_TESTS:.cpp=.test)

all: $(OBJECTS_TESTS)

$(SDIR)/%.test: $(SDIR)/%.cpp
	$(CXX) $(CXX_STD) $(OPT_FLAGS) -I$(HEADER_DIR) $< -o $@ $(LIBS)

# cleanup
.PHONY: clean
clean:
	@rm -rf *.test *.gcov *.gcno *.gcda *.dSYM

# individual make calls

GCEM_MAKE_CALL= $(CXX) $(CXX_STD) $(OPT_FLAGS) -I$(HEADER_DIR) $@.cpp -o $@.test $(LIBS)

abs:
	$(GCEM_MAKE_CALL)

acos:
	$(GCEM_MAKE_CALL)

acosh:
	$(GCEM_MAKE_CALL)

asin:
	$(GCEM_MAKE_CALL)

asinh:
	$(GCEM_MAKE_CALL)

atan:
	$(GCEM_MAKE_CALL)

atan2:
	$(GCEM_MAKE_CALL)

atanh:
	$(GCEM_MAKE_CALL)

binomial_coef:
	$(GCEM_MAKE_CALL)

copysign:
	$(GCEM_MAKE_CALL)

cos:
	$(GCEM_MAKE_CALL)

cosh:
	$(GCEM_MAKE_CALL)

erf:
	$(GCEM_MAKE_CALL)

erf_inv:
	$(GCEM_MAKE_CALL)

exp:
	$(GCEM_MAKE_CALL)

expm1:
	$(GCEM_MAKE_CALL)

fabs:
	$(GCEM_MAKE_CALL)

fabsf:
	$(GCEM_MAKE_CALL)

fabsl:
	$(GCEM_MAKE_CALL)

factorial:
	$(GCEM_MAKE_CALL)

fmod:
	$(GCEM_MAKE_CALL)

gcd:
	$(GCEM_MAKE_CALL)

hypot:
	$(GCEM_MAKE_CALL)

incomplete_beta:
	$(GCEM_MAKE_CALL)

incomplete_beta_inv:
	$(GCEM_MAKE_CALL)

incomplete_gamma:
	$(GCEM_MAKE_CALL)

incomplete_gamma_inv:
	$(GCEM_MAKE_CALL)

inv_sqrt:
	$(GCEM_MAKE_CALL)

is_odd:
	$(GCEM_MAKE_CALL)

lcm:
	$(GCEM_MAKE_CALL)

lgamma:
	$(GCEM_MAKE_CALL)

log_binomial_coef:
	$(GCEM_MAKE_CALL)

log:
	$(GCEM_MAKE_CALL)

log1p:
	$(GCEM_MAKE_CALL)

log2:
	$(GCEM_MAKE_CALL)

log10:
	$(GCEM_MAKE_CALL)

other:
	$(GCEM_MAKE_CALL)

pow:
	$(GCEM_MAKE_CALL)

rounding:
	$(GCEM_MAKE_CALL)

signbit:
	$(GCEM_MAKE_CALL)

sin:
	$(GCEM_MAKE_CALL)

sinh:
	$(GCEM_MAKE_CALL)

sqrt:
	$(GCEM_MAKE_CALL)

tan:
	$(GCEM_MAKE_CALL)

tanh:
	$(GCEM_MAKE_CALL)

tgamma:
	$(GCEM_MAKE_CALL)
