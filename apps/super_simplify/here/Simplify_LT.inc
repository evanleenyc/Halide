((b.node_type() == IRNodeType::Add) && (
 rewrite(x < (y + 1), x <= y) ||
((a.node_type() == IRNodeType::Add) && EVAL_IN_LAMBDA(
 rewrite((x + y) < ((z + (w + y)) + u), x < ((z + w) + u)) ||
 rewrite((x + y) < (((y + z) + w) + u), x < ((z + w) + u)) ||
 rewrite((x + y) < (((z + y) + w) + u), x < ((z + w) + u)) ||
 rewrite((((x + y) + z) + w) < (u + y), ((x + z) + w) < u))))) ||
((a.node_type() == IRNodeType::Min) && EVAL_IN_LAMBDA(
((b.node_type() == IRNodeType::Min) && (
 rewrite(min(x, y) < min(min(y, z), w), x < min(min(y, z), w)) ||
 rewrite(min(x, y) < min(min(z, y), w), x < min(min(z, y), w)) ||
 rewrite(min(min(x, y), z) < min(w, y), min(x, z) < min(w, y))))))
