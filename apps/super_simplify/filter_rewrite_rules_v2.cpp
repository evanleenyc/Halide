#include "Halide.h"
#include "expr_util.h"
#include "parser.h"
#include "reduction_order.h"
#include "synthesize_predicate.h"
#include "z3.h"

#include <fstream>

using namespace Halide;
using namespace Halide::Internal;

// Take a list of rewrite rules and classify them by root IR node, and
// what problems they might have that require further investigation.

struct Rule {
    Expr lhs, rhs, predicate;
    Expr orig;
    bool has_pred;
};
std::ostream &operator<<(std::ostream &stream, const IRNodeType &irtype) {
    std::string out;
    switch(irtype) {
        case IRNodeType::Add:
            out = "Add";
            break;
        case IRNodeType::And:
            out = "And";
            break;
        case IRNodeType::Div:
            out = "Div";
            break;
        case IRNodeType::EQ:
            out = "EQ";
            break;
        case IRNodeType::LE:
            out = "LE";
            break;
        case IRNodeType::LT:
            out = "LT";
            break;
        case IRNodeType::Max:
            out = "Max";
            break;
        case IRNodeType::Min:
            out = "Min";
            break;
        case IRNodeType::Mod:
            out = "Mod";
            break;
        case IRNodeType::Mul:
            out = "Mul";
            break;
        case IRNodeType::Or:
            out = "Or";
            break;
        case IRNodeType::Select:
            out = "Select";
            break;
        case IRNodeType::Sub:
            out = "Sub";
            break;
        default:
            out = "UNDEFINED";
            break;
    }
    stream << out;
    return stream;
}

using std::map;
using std::set;
using std::string;
using std::vector;

// Canonicalize ordering of children in commutative ops
class Canonicalizer : public IRMutator {
    using IRMutator::visit;

    template<typename Op>
    Expr visit_commutative_op(const Op *op) {
        Expr a = mutate(op->a);
        Expr b = mutate(op->b);
        IRNodeType a_node_type = a.node_type();
        IRNodeType b_node_type = b.node_type();
        const Call *call_a = a.as<Call>();
        const Call *call_b = b.as<Call>();
        const Variable *var_a = a.as<Variable>();
        const Variable *var_b = b.as<Variable>();
        bool a_is_const = (is_const(a) ||
                           (var_a && var_a->name[0] == 'c') ||
                           (call_a && call_a->name == "fold"));
        if (a_is_const) {
            a_node_type = IRNodeType::IntImm;
        }
        bool b_is_const = (is_const(b) ||
                           (var_b && var_b->name[0] == 'c') ||
                           (call_b && call_b->name == "fold"));
        if (b_is_const) {
            b_node_type = IRNodeType::IntImm;
        }
        bool should_commute = ((a_is_const && !b_is_const) ||
                               (!var_a && !var_b && a_node_type < b_node_type));
        if (should_commute) {
            return Op::make(b, a);
        } else {
            return Op::make(a, b);
        }
    }

    Expr visit(const Add *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const Mul *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const Min *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const Max *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const And *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const Or *op) override {
        return visit_commutative_op(op);
    }
    Expr visit(const EQ *op) override {
        return visit_commutative_op(op);
    }
};

// Levenshtein distance algorithm copied from wikipedia
unsigned int edit_distance(const std::string &s1, const std::string &s2) {
    const std::size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

    d[0][0] = 0;
    for (unsigned int i = 1; i <= len1; ++i)
        d[i][0] = i;
    for (unsigned int i = 1; i <= len2; ++i)
        d[0][i] = i;

    for (unsigned int i = 1; i <= len1; ++i)
        for (unsigned int j = 1; j <= len2; ++j)
            // note that std::min({arg1, arg2, arg3}) works only in C++11,
            // for C++98 use std::min(std::min(arg1, arg2), arg3)
            d[i][j] =
                std::min({d[i - 1][j] + 1,
                          d[i][j - 1] + 1,
                          d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
    return d[len1][len2];
}

std::string expr_to_rpn_string(const Expr &e) {
    class VisitLeaves : public IRMutator {
        using IRMutator::visit;
        Expr visit(const Variable *op) override {
            ss << op->name;
            return op;
        }

    public:
        using IRMutator::mutate;
        Expr mutate(const Expr &e) override {
            if (is_const(e) || e.as<Variable>()) {
                IRMutator::mutate(e);
            } else {
                ss << "(";
                IRMutator::mutate(e);
                ss << ")";
            }
            return e;
        }

        std::stringstream ss;
    } visit_leaves;
    visit_leaves.mutate(e);
    return visit_leaves.ss.str();
}

vector<Rule> generate_commuted_variants(const Rule &rule) {
    vector<Expr> lhs = generate_commuted_variants(rule.lhs);
    vector<Expr> rhs = generate_reassociated_variants(rule.rhs);

    vector<Rule> result;
    for (Expr l : lhs) {
        l = Canonicalizer().mutate(l);
        Rule r2 = rule;
        r2.lhs = l;
        // Pick the rhs that minimizes edit distance
        std::string lhs_str = expr_to_rpn_string(l);
        int best_edit_distance = -1;
        for (Expr r : rhs) {
            r = Canonicalizer().mutate(r);
            std::string rhs_str = expr_to_rpn_string(r);
            int d = edit_distance(lhs_str, rhs_str);
            if (best_edit_distance < 0 || d < best_edit_distance) {
                r2.rhs = r;
                best_edit_distance = d;
            }
        }
        result.push_back(r2);
    }
    return result;
}

Expr remove_folds(const Expr &e) {
    class RemoveFolds : public IRMutator {
        using IRMutator::visit;

        Expr visit(const Call *op) override {
            if (op->name == "fold") {
                return op->args[0];
            } else {
                return IRMutator::visit(op);
            }
        }
    };

    return RemoveFolds().mutate(e);
}

Expr inject_folds(const Expr &e) {

    class InjectFolds : public IRMutator {
        bool constant = false;

        using IRMutator::visit;

        Expr visit(const Variable *var) override {
            if (var->name[0] != 'c') {
                constant = false;
            }
            return var;
        }

    public:
        using IRMutator::mutate;

        Expr mutate(const Expr &e) override {
            bool old = constant;
            constant = true;
            Expr new_e = IRMutator::mutate(e);
            if (constant) {
                // Note we wrap a fold around the *unmutated* child,
                // to avoid nested folds.
                constant = constant && old;
                if (is_const(e) || e.as<Variable>()) {
                    return e;
                } else {
                    return Call::make(e.type(), "fold", {e}, Call::PureExtern);
                }
            } else {
                constant = constant && old;
                return new_e;
            }
        }
    };

    return InjectFolds().mutate(e);
}

class ImplicitPredicate : public IRVisitor {
    using IRVisitor::visit;

    void visit(const Div *op) {
        const Variable *v = op->b.as<Variable>();
        if (v && v->name[0] == 'c') {
            // Legal, but would have folded
            result = result && (op->b != 0) && (op->b != 1) && (op->b != -1);
        }
        IRVisitor::visit(op);
    }

    void visit(const Mul *op) {
        const Variable *v = op->b.as<Variable>();
        if (v && v->name[0] == 'c') {
            // Would have folded
            result = result && (op->b != 0) && (op->b != 1);
        }
        IRVisitor::visit(op);
    }

    void visit(const Mod *op) {
        const Variable *v = op->b.as<Variable>();
        if (v && v->name[0] == 'c') {
            // Would have folded
            result = result && (op->b != 0) && (op->b != 1) && (op->b != -1);
        }
        IRVisitor::visit(op);
    }

public:
    ImplicitPredicate()
        : result(const_true()) {
    }
    Expr result;
};

class MoveNegationInnermost : public IRMutator {
    using IRMutator::visit;

    Expr visit(const Not *op) override {
        if (const And *and_a = op->a.as<And>()) {
            return mutate(!and_a->a) || mutate(!and_a->b);
        } else if (const Or *or_a = op->a.as<Or>()) {
            return mutate(!or_a->a) && mutate(!or_a->b);
        } else if (const Not *not_a = op->a.as<Not>()) {
            return mutate(not_a->a);
        } else if (const LT *lt = op->a.as<LT>()) {
            return mutate(lt->b <= lt->a);
        } else if (const LE *le = op->a.as<LE>()) {
            return mutate(le->b < le->a);
        } else if (const EQ *eq = op->a.as<EQ>()) {
            return mutate(eq->a != eq->b);
        } else if (const NE *ne = op->a.as<NE>()) {
            return mutate(ne->a == ne->b);
        } else {
            return IRMutator::visit(op);
        }
    }
};

class ToDNF : public IRMutator {
    using IRMutator::visit;

    Expr visit(const And *op) override {
        Expr a = mutate(op->a);
        Expr b = mutate(op->b);
        vector<Expr> as = unpack_binary_op<Or>(a);
        vector<Expr> bs = unpack_binary_op<Or>(b);
        set<Expr, IRDeepCompare> result;
        for (Expr a1 : as) {
            for (Expr b1 : bs) {
                auto a_clauses = unpack_binary_op<And>(a1);
                auto b_clauses = unpack_binary_op<And>(b1);
                set<Expr, IRDeepCompare> both;
                both.insert(a_clauses.begin(), a_clauses.end());
                both.insert(b_clauses.begin(), b_clauses.end());
                result.insert(pack_binary_op<And>(both));
            }
        }
        return pack_binary_op<Or>(result);
    }

    Expr visit(const EQ *op) override {
        if (op->a.type().is_bool()) {
            return mutate((op->a && op->b) || (!op->a && !op->b));
        } else {
            return IRMutator::visit(op);
        }
    }

    Expr visit(const LE *op) override {
        if (const Min *min_a = op->a.as<Min>()) {
            return mutate(min_a->a <= op->b || min_a->b <= op->b);
        } else if (const Max *max_a = op->a.as<Max>()) {
            return mutate(max_a->a <= op->b && max_a->b <= op->b);
        } else if (const Min *min_b = op->b.as<Min>()) {
            return mutate(op->a <= min_b->a && op->a <= min_b->b);
        } else if (const Max *max_b = op->b.as<Max>()) {
            return mutate(op->a <= max_b->a || op->a <= max_b->b);
        } else {
            return IRMutator::visit(op);
        }
    }

    Expr visit(const LT *op) override {
        if (const Min *min_a = op->a.as<Min>()) {
            return mutate(min_a->a < op->b || min_a->b < op->b);
        } else if (const Max *max_a = op->a.as<Max>()) {
            return mutate(max_a->a < op->b && max_a->b < op->b);
        } else if (const Min *min_b = op->b.as<Min>()) {
            return mutate(op->a < min_b->a && op->a < min_b->b);
        } else if (const Max *max_b = op->b.as<Max>()) {
            return mutate(op->a < max_b->a || op->a < max_b->b);
        } else {
            return IRMutator::visit(op);
        }
    }
};

// Make the first wildcard found x, the second y, etc.
class CanonicalizeVariableNames : public IRMutator {
    map<string, string> remapping;
    const char *wild_names[6] = {"x", "y", "z", "w", "u", "v"};
    int next_wild = 0;
    int next_constant = 0;

    using IRMutator::visit;

    Expr visit(const Variable *op) override {
        auto it = remapping.find(op->name);
        if (it != remapping.end()) {
            return Variable::make(op->type, it->second);
        } else {
            std::string n;
            if (op->name[0] == 'c') {
                n = "c" + std::to_string(next_constant);
                next_constant++;
            } else {
                assert(next_wild < 6);
                n = wild_names[next_wild];
                next_wild++;
            }
            remapping[op->name] = n;
            return Variable::make(op->type, n);
        }
    }
};

void replace_all(string &str, const string &from, const string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

void check_rule(Rule &r) {
    // Check the rules with Z3
    map<string, Expr> mapping;
    ImplicitPredicate imp;
    r.lhs.accept(&imp);
    if (!is_const_zero(r.predicate)) {
        auto result = satisfy(r.predicate && imp.result && r.lhs != r.rhs, &mapping);
        if (result == Z3Result::Unsat) {
            std::cout << "Verified with SMT: rewrite("
                      << r.lhs << ", " << r.rhs << ", " << r.predicate << ")\n";
            return;
        } else if (result == Z3Result::Sat) {
            std::cout << "Incorrect rule: rewrite("
                      << r.lhs << ", " << r.rhs << ", " << r.predicate << ")\n"
                      << "Counterexample is: ";
            for (auto p : mapping) {
                std::cout << p.first << " = " << p.second << "\n";
            }
            std::cout << "For counterexample, LHS = " << simplify(substitute(mapping, r.lhs))
                      << " RHS = " << simplify(substitute(mapping, r.rhs)) << "\n";
        } else if (result == Z3Result::Unknown) {
            std::cout << "Z3 returned unknown/timeout for: rewrite("
                      << r.lhs << ", " << r.rhs << ", " << r.predicate << ")\n";
        }
    }

    map<string, Expr> binding;
    if (is_const_zero(r.predicate)) {
        std::cout << "Re-synthesizing predicate for " << r.orig << " with a larger beam size\n";
        Expr new_predicate = const_false();
        if (false) {
            int bs = 1;
            while (bs <= 16 && is_const_zero(new_predicate)) {
                std::cout << "Trying with beam size: " << bs << "\n";
                binding.clear();
                new_predicate = synthesize_predicate(r.lhs, r.rhs, &binding, bs);
                bs *= 4;
            }
        }

        if (is_const_zero(new_predicate)) {
            // Ok so that didn't work so well. Let's try an
            // alternative algorithm for predicate synthesis.
            new_predicate = const_true();

            Expr rule_holds = simplify(r.lhs == r.rhs);
            debug(0) << "Rule holds: " << rule_holds << "\n";

            // We can substitute in any old values for the
            // non-constant variables to get a candidate
            // constraint. Let's start with 0/1

            auto vars = find_vars(rule_holds);

            map<string, Expr> all_vars_zero;
            for (const auto &p : vars) {
                if (p.first[0] == 'c') continue;
                all_vars_zero.emplace(p.first, cast(p.second.first.type(), 0));
            }
            vector<Expr> terms;
            terms.push_back(substitute(all_vars_zero, rule_holds));
            for (const auto &p : vars) {
                if (p.first[0] == 'c') continue;
                all_vars_zero[p.first] = cast(p.second.first.type(), 1);
                terms.push_back(simplify(substitute(all_vars_zero, rule_holds)));
                all_vars_zero[p.first] = cast(p.second.first.type(), 0);
            }

            new_predicate = pack_binary_op<And>(terms);

            // Exploit the implicit predicate to clean some terms up
            {
                Simplify simplifier(true, nullptr, nullptr);
                auto scoped_fact = simplifier.scoped_truth(imp.result);
                // simplifier.learn_true(imp.result);
                for (auto p : find_vars(new_predicate)) {
                    if (p.first[0] == 'c') {
                        Expr v = p.second.first;
                        if (is_const_zero(simplifier.mutate(v == -1 || v == 0 || v == 1, nullptr))) {
                            // This var appears on the RHS of a div or mod
                            new_predicate = substitute(1 % v, 1, new_predicate);
                            new_predicate = substitute(1 / v, 0, new_predicate);
                        }
                        new_predicate = substitute(-1 / v == 0, v == 0, new_predicate);
                        new_predicate = substitute(-1 / v == -1, 0 < v, new_predicate);
                        new_predicate = substitute(-1 / v == 1, v < 0, new_predicate);
                    }
                }
                new_predicate = simplifier.mutate(new_predicate, nullptr);
            }

            auto lhs_vars = find_vars(r.lhs);
            for (int terms = 0;; terms++) {
                // Try to eliminate constant vars that only occur on the RHS
                {
                    for (auto v : find_vars(r.rhs)) {
                        if (lhs_vars.count(v.first)) continue;
                        for (Expr t : unpack_binary_op<And>(new_predicate)) {
                            auto result = solve_expression(t, v.first);
                            if (result.fully_solved) {
                                if (const EQ *eq = result.result.as<EQ>()) {
                                    if (equal(eq->a, Variable::make(Int(32), v.first))) {
                                        Expr replacement = simplify(eq->b);
                                        new_predicate = simplify(substitute(v.first, replacement, new_predicate));
                                        r.rhs = substitute(v.first, replacement, r.rhs);
                                        rule_holds = simplify(substitute(v.first, replacement, rule_holds));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                if (terms > 4) {
                    // May be trying to handle an infinite number of cases one term at a time
                    debug(0) << "Giving up. Accumulating too many terms\n";

                    new_predicate = MoveNegationInnermost().mutate(new_predicate);
                    new_predicate = ToDNF().mutate(new_predicate);
                    set<Expr, IRDeepCompare> clauses;
                    for (auto clause : unpack_binary_op<Or>(new_predicate)) {
                        clause = simplify(clause);
                        if (is_const_zero(clause)) continue;
                        clauses.insert(clause);
                    }

                    debug(0) << "Predicate in DNF form:\n";
                    for (auto c : clauses) {
                        debug(0) << " " << c << "\n";
                    }

                    // Right now we have a necessary condition which
                    // is a disjunction (i.e. union) of a bunch of
                    // clauses. Try to find a subset of the clauses
                    // which we can prove are sufficient conditions,
                    // and just keep those.
                    set<Expr, IRDeepCompare> trimmed_clauses;
                    bool any_timeouts = false;
                    for (auto c : clauses) {

                        // Aggressively simplify the clause
                        auto terms = unpack_binary_op<And>(c);
                        for (size_t i = 0; i < terms.size(); i++) {
                            Simplify simplifier(true, nullptr, nullptr);
                            auto scoped_fact_res = simplifier.scoped_truth(imp.result);
                            auto scoped_fact_terms = simplifier.scoped_truth(terms[i]);
                            // simplifier.learn_true(imp.result);
                            // simplifier.learn_true(terms[i]);
                            for (size_t j = 0; j < terms.size(); j++) {
                                if (i == j) continue;
                                Simplify::ExprInfo info;
                                terms[j] = simplifier.mutate(terms[j], &info);
                            }
                        }
                        c = pack_binary_op<And>(terms);

                        map<string, Expr> binding;
                        auto z3_result = satisfy(imp.result && c && !rule_holds, &binding, "checking one clause in DNF", 30);
                        if (z3_result == Z3Result::Sat) continue;
                        any_timeouts |= (z3_result != Z3Result::Unsat);
                        trimmed_clauses.insert(c);
                    }
                    trimmed_clauses.insert(const_false());

                    new_predicate = simplify(pack_binary_op<Or>(trimmed_clauses));
                    if (any_timeouts && !is_const_zero(new_predicate)) {
                        new_predicate = Call::make(Bool(), "prove_me", {new_predicate}, Call::Extern);
                    }
                    break;
                }
                Expr there_is_a_failure = simplify(imp.result && new_predicate && !rule_holds);
                map<string, Expr> binding;
                auto z3_result = satisfy(there_is_a_failure, &binding, "checking a predicate for failures", 30);
                if (z3_result == Z3Result::Unsat) {
                    // Woo. No failures exist.
                    break;
                } else if (z3_result == Z3Result::Sat) {
                    Expr new_term = rule_holds;
                    for (auto p : binding) {
                        if (p.first[0] != 'c') {
                            new_term = substitute(p.first, p.second, new_term);
                        }
                    }
                    debug(0) << "new_term: " << new_term << "\n";
                    new_term = simplify(new_term);
                    new_predicate = new_predicate && new_term;
                    debug(0) << "new_predicate: " << new_predicate << "\n";
                    new_predicate = simplify(new_predicate);
                } else {
                    // Couldn't find a failure, so hopefully
                    // there aren't any. Would Require human
                    // checking though.
                    debug(0) << "Z3 Timeout\n";
                    if (false && can_disprove_nonconvex(there_is_a_failure, 256, nullptr)) {
                        debug(0) << "Verified using beam search\n";
                    } else {
                        // A human will have to prove this by hand
                        new_predicate = Call::make(Bool(), "prove_me", {new_predicate}, Call::Extern);
                    }
                    break;
                }
            }
            if (!is_const_zero(new_predicate)) {
                debug(0) << "\n\nNew predicate synthesis algorithm produced: " << new_predicate << "\n\n\n";
            }
        }

        // Eliminate constant vars that occur on the LHS
        {
            for (auto v : find_vars(r.lhs)) {
                for (Expr t : unpack_binary_op<And>(new_predicate)) {
                    auto result = solve_expression(t, v.first);
                    if (result.fully_solved) {
                        if (const EQ *eq = result.result.as<EQ>()) {
                            if (equal(eq->a, Variable::make(Int(32), v.first))) {
                                Expr replacement = simplify(eq->b);
                                const Variable *r_var = replacement.as<Variable>();
                                bool lower_numbered_constant_var =
                                    r_var && (r_var->name[0] == 'c' && (r_var->name[1] < v.first[1]));
                                if (lower_numbered_constant_var || is_const(replacement)) {
                                    new_predicate = simplify(substitute(v.first, replacement, new_predicate));
                                    r.lhs = substitute(v.first, replacement, r.lhs);
                                    r.rhs = substitute(v.first, replacement, r.rhs);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Save human attention for things small enough to be tractable - one clause only please.
        if (const Call *c = new_predicate.as<Call>()) {
            if (c->name == "prove_me") {
                if (c->args[0].as<And>()) {
                    new_predicate = const_false();
                }
            }
        }

        if (!can_prove(r.predicate == new_predicate)) {
            std::cout << "Rewrote predicate: " << r.predicate << " -> " << new_predicate << "\n";
            r.predicate = new_predicate;
        }
        r.lhs = substitute(binding, r.lhs);

        for (auto &it : binding) {
            it.second = Call::make(it.second.type(), "fold", {it.second}, Call::PureExtern);
        }
        r.rhs = substitute(binding, r.rhs);
    }
}

class UniqueConstantVars : public IRMutator {
    using IRMutator::visit;

    int next = 0;

    Expr visit(const Variable *op) override {
        if (op->name[0] == 'c') {
            return Variable::make(op->type, "c" + std::to_string(next++));
        } else {
            return op;
        }
    }

    Expr visit(const Call *op) override {
        if (op->name == "fold") {
            return Variable::make(op->type, "c" + std::to_string(next++));
        } else {
            return IRMutator::visit(op);
        }
    }
};

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: ./filter_rewrite_rules rewrite_rules.txt [output_dir]\n";
        return 0;
    }

    const string rewrite_rules_path = argv[1];
    string output_dir_path = argc >= 3 ? argv[2] : "";
    if (output_dir_path.empty()) {
        output_dir_path = ".";
    }
    if (output_dir_path[output_dir_path.size() - 1] != '/') {
        output_dir_path += "/";
    }
    debug(0) << "output path is " << output_dir_path << "\n";

    vector<Expr> exprs_vec = parse_halide_exprs_from_file(rewrite_rules_path);

    // De-dup
    std::set<Expr, IRDeepCompare> exprs;
    exprs.insert(exprs_vec.begin(), exprs_vec.end());

    vector<Rule> rules;

    // vector<Expr> generalized;
    // for (Expr e : exprs) {
    //     // Add a variant with all constants unique
    //     // coincidence in the original rule.
    //     Expr new_e = UniqueConstantVars().mutate(e);
    //     if (!equal(new_e, e)) {
    //         generalized.push_back(new_e);
    //     }
    // }
    // exprs.insert(generalized.begin(), generalized.end());

    for (Expr e : exprs) {
        if (const Call *call = e.as<Call>()) {
            if (call->name != "rewrite") {
                std::cerr << "Expr is not a rewrite rule: " << e << "\n";
                return -1;
            }

            if (call->args.size() == 3) {
                rules.emplace_back(Rule{call->args[0], call->args[1], call->args[2], e, true});
            } else if (call->args.size() == 2) {
                rules.emplace_back(Rule{call->args[0], call->args[1], const_true(), e, false});
            } else {
                std::cerr << "Expr is not a rewrite rule (bad arg count): " << e << "\n";
                return -1;
            }
        } else {
            std::cerr << "Expr is not a rewrite rule: " << e << "\n";
            return -1;
        }
    }

    class TopLevelNodeTypes : public IRMutator {
        int depth = 0;

    public:
        using IRMutator::mutate;
        Expr mutate(const Expr &e) {
            if (depth >= 2) {
                return e;
            }
            result.push_back(e.node_type());
            depth++;
            IRMutator::mutate(e);
            depth--;
            return e;
        }
        vector<IRNodeType> result;
    };

    // Sort the rules by LHS
    std::sort(rules.begin(), rules.end(),
              [](const Rule &r1, const Rule &r2) {
                  TopLevelNodeTypes t1, t2;
                  t1.mutate(r1.lhs);
                  t2.mutate(r2.lhs);
                  if (t1.result.size() < t2.result.size()) {
                      return true;
                  }
                  if (t2.result.size() < t1.result.size()) {
                      return false;
                  }
                  for (size_t i = 0; i < t1.result.size(); i++) {
                      if (t1.result[i] < t2.result[i]) {
                          return true;
                      }
                      if (t2.result[i] < t1.result[i]) {
                          return false;
                      }
                  }
                  if (IRDeepCompare{}(r1.lhs, r2.lhs)) {
                      return true;
                  }
                  if (IRDeepCompare{}(r2.lhs, r1.lhs)) {
                      return false;
                  }
                  return IRDeepCompare{}(r1.predicate, r2.predicate);
              });

    std::map<IRNodeType, vector<Rule>> good_ones;

    for (const Rule &r : rules) {
        good_ones[r.lhs.node_type()].push_back(r);
    }

    std::cout << "Generated rules:\n";
    for (auto it : good_ones) {
        std::cout << "Simplify_" << it.first << ".inc:\n";
        std::ostringstream os;
        IRNodeType last_a_type = IRNodeType::Variable, last_b_type = IRNodeType::Variable;
        bool first_line = true;
        (void)first_line;
        for (auto r : it.second) {
            TopLevelNodeTypes t;
            t.mutate(r.lhs);
            IRNodeType a_type = t.result.size() > 0 ? t.result[1] : IRNodeType::Variable;
            IRNodeType b_type = t.result.size() > 1 ? t.result[2] : IRNodeType::Variable;

            if (a_type != last_a_type && last_a_type != IRNodeType::Variable) {
                if (last_b_type != IRNodeType::Variable) {
                    // Close out final b group in the a bucket
                    os << "))";
                    last_b_type = IRNodeType::Variable;
                }
                // Close the a bucket
                os << "))";
                last_a_type = IRNodeType::Variable;
            } else if (b_type != last_b_type && last_b_type != IRNodeType::Variable) {
                // Same a group, new b bucket
                os << "))";
                last_b_type = IRNodeType::Variable;
            }

            if (!first_line) {
                os << " ||\n";
            }
            first_line = false;

            if (a_type != last_a_type && a_type != IRNodeType::Variable) {
                // Open a new a bucket
                os << "((a.node_type() == IRNodeType::" << a_type << ") && EVAL_IN_LAMBDA(\n";
            }

            if (b_type != last_b_type && b_type != IRNodeType::Variable) {
                // open a new b type bucket
                os << "((b.node_type() == IRNodeType::" << b_type << ") && (\n";
            }

            last_a_type = a_type;
            last_b_type = b_type;

            vector<Expr> args = {r.lhs, r.rhs};
            if (!is_const_one(r.predicate)) {
                args.push_back(r.predicate);
            }
            Expr rule_expr = Call::make(Int(32), "rewrite", args, Call::Extern);

            os << " " << rule_expr;
        }

        if (last_b_type != IRNodeType::Variable) {
            os << "))";
        }

        if (last_a_type != IRNodeType::Variable) {
            os << "))";
        }

        os << "\n";

        std::cout << os.str();

        std::ostringstream filename;
        filename << output_dir_path << "Simplify_" << it.first << ".inc";
        std::ofstream of;
        of.open(filename.str());
        if (of.fail()) {
            debug(0) << "Unable to open " << filename.str();
            assert(false);
        }

        // Clean up bool terms that aren't valid C++ in the simplifier
        string s = os.str();
        replace_all(s, "(uint1)0", "false");
        replace_all(s, "(uint1)1", "true");
        replace_all(s, "prove_me(true)", "prove_me(IRMatcher::Const(1))");
        replace_all(s, "(uint1)", "");
        of << s;
        of.close();
    }

    // Make sure we write a complete set of .inc files, to avoid
    // accidentally mixing and matching between experiments.
    for (IRNodeType t : {
             IRNodeType::Add,
             IRNodeType::And,
             IRNodeType::Div,
             IRNodeType::EQ,
             IRNodeType::LE,
             IRNodeType::LT,
             IRNodeType::Max,
             IRNodeType::Min,
             IRNodeType::Mod,
             IRNodeType::Mul,
             IRNodeType::Or,
             IRNodeType::Select,
             IRNodeType::Sub}) {
        if (good_ones.count(t) == 0) {
            std::ostringstream filename;
            filename << output_dir_path << "Simplify_" << t << ".inc";
            std::ofstream of;
            of.open(filename.str());
            if (of.fail()) {
                debug(0) << "Unable to open " << filename.str();
                assert(false);
            }
            of << "false";
            of.close();
        }
    }

    return 0;
}
