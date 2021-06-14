((b.node_type() == IRNodeType::Not) && (
 rewrite.debug_new((x || !(x && y)), true) ||
 rewrite.debug_new((x || !(y && x)), true))) ||
((a.node_type() == IRNodeType::And) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::NE) && (
 rewrite.debug_new(((x && (y == z)) || (y != z)), (x || (z != y))) ||
 rewrite.debug_new(((x && (y == z)) || (z != y)), (x || (y != z))) ||
 rewrite.debug_new((((x == y) && z) || (x != y)), (z || (y != x))) ||
 rewrite.debug_new((((x == y) && z) || (y != x)), ((x != y) || z)))))) ||
((a.node_type() == IRNodeType::Or) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::LT) && (
 rewrite.debug_new(((x || (y < z)) || (w < z)), (x || (min(y, w) < z))) ||
 rewrite.debug_new(((x || (y < z)) || (y < (w + u))), (x || (y < max(z, (w + u))))) ||
 rewrite.debug_new(((x || (y < (z + w))) || (y < u)), (x || (y < max((z + w), u)))) ||
 rewrite.debug_new(((x || (y || (z < w))) || (u < w)), (x || ((min(z, u) < w) || y))) ||
 rewrite.debug_new(((x || (y || (z || (w < u)))) || (v < u)), (x || (y || (z || (min(w, v) < u))))) ||
 rewrite.debug_new(((x || (y || ((z < w) || u))) || (v < w)), ((x || (y || (min(z, v) < w))) || u)) ||
 rewrite.debug_new(((x || ((y < z) || w)) || (u < z)), ((x || (min(y, u) < z)) || w)) ||
 rewrite.debug_new(((x || ((y || (z < w)) || u)) || (v < w)), ((x || (min(z, v) < w)) || (u || y))) ||
 rewrite.debug_new(((x || (((y < z) || w) || u)) || (v < z)), ((x || ((min(y, v) < z) || u)) || w)) ||
 rewrite.debug_new((((x < y) || z) || (w < y)), (z || (min(x, w) < y))) ||
 rewrite.debug_new((((x < y) || z) || (x < (w + u))), (z || (x < max(y, (w + u))))) ||
 rewrite.debug_new((((x < (y + z)) || w) || (x < u)), (w || (x < max((z + y), u)))) ||
 rewrite.debug_new((((x || (y < z)) || w) || (u < z)), (((min(y, u) < z) || w) || x)) ||
 rewrite.debug_new((((x || (y || (z < w))) || u) || (v < w)), (y || (x || ((min(z, v) < w) || u)))) ||
 rewrite.debug_new((((x || ((y < z) || w)) || u) || (v < z)), (((x || (min(y, v) < z)) || u) || w)) ||
 rewrite.debug_new(((((x < y) || z) || w) || (u < y)), (((min(x, u) < y) || w) || z)) ||
 rewrite.debug_new(((((x || (y < z)) || w) || u) || (v < z)), (((min(y, v) < z) || u) || (w || x))) ||
 rewrite.debug_new((((((x < y) || z) || w) || u) || (v < y)), ((((min(x, v) < y) || w) || u) || z)))) ||
((b.node_type() == IRNodeType::LE) && (
 rewrite.debug_new(((x || (y || (z <= w))) || (u <= w)), (x || ((min(z, u) <= w) || y))) ||
 rewrite.debug_new(((x || ((y <= z) || w)) || (u <= z)), ((x || (min(y, u) <= z)) || w)) ||
 rewrite.debug_new((((x || (y <= z)) || w) || (u <= z)), (((min(y, u) <= z) || w) || x)) ||
 rewrite.debug_new(((((x <= y) || z) || w) || (u <= y)), (((min(x, u) <= y) || w) || z)))))) ||
((a.node_type() == IRNodeType::Not) && EVAL_IN_LAMBDA(
 rewrite.debug_new((!(x && y) || x), true) ||
 rewrite.debug_new((!(x && y) || y), true) ||
((b.node_type() == IRNodeType::EQ) && (
 rewrite.debug_new((!(x && (y == z)) || (z == y)), true) ||
 rewrite.debug_new((!((x == y) && z) || (y == x)), true))) ||
((b.node_type() == IRNodeType::NE) && (
 rewrite.debug_new((!(x && (y == z)) || (y != z)), !(x && (z == y))) ||
 rewrite.debug_new((!(x && (y == z)) || (z != y)), !(x && (z == y))) ||
 rewrite.debug_new((!((x == y) && z) || (x != y)), !((x == y) && z)) ||
 rewrite.debug_new((!((x == y) && z) || (y != x)), !((y == x) && z))))))
