#pragma once

namespace worth { namespace protocol {

enum curve_id
{
   quadratic,
   bounded_curation,
   linear,
   square_root,
   convergent_linear,
   convergent_square_root
};

} } // worth::utilities


FC_REFLECT_ENUM(
   worth::protocol::curve_id,
   (quadratic)
   (bounded_curation)
   (linear)
   (square_root)
   (convergent_linear)
   (convergent_square_root)
)
