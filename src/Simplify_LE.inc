((b.node_type() == IRNodeType::Select) && (
 rewrite.debug_new((x <= select(y, x, z)), (y || (x <= z))))) ||
((a.node_type() == IRNodeType::Select) && EVAL_IN_LAMBDA(
 rewrite.debug_new((select(x, y, z) <= y), ((z <= y) || x))))
