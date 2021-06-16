((a.node_type() == IRNodeType::Or) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::LT) && (
 rewrite(((x < y) || z) || (w < y), z || (min(x, w) < y)))) ||
((b.node_type() == IRNodeType::LE) && (
 rewrite(((x || (y <= z)) || w) || (u <= z), ((min(y, u) <= z) || w) || x)))))
