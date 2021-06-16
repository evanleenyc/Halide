((a.node_type() == IRNodeType::Select) && EVAL_IN_LAMBDA(
 rewrite(select(x, 1, 0)*y, select(x, y, 0))))
