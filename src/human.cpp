#include <cassert>

#include "human.h"
#include "utils.h"

// Given a target square for castling, return whether it is kingside.
inline bool is_kingside_castle(Square target_sq) {
    return target_sq & 4;
}

inline char piece_char(PieceType piece) {
    switch (piece) {
        case PAWN:
            return 'P';
        case KNIGHT:
            return 'N';
        case BISHOP:
            return 'B';
        case ROOK:
            return 'R';
        case KING:
            return 'K';
        default: /* queen */
            return 'Q';
    }
}

string human::pretty_move(Move mv, const std::vector<Move>& legal_moves, const Position& pos, bool checking, bool mating) {
    char buf[8] = "O-O\0\0\0\0"; // kingside castle by default
    int bufidx = 0;
    MoveType mt = move_type(mv);
    Square tgt_sq = move_target(mv);
    if (mt == NORMAL_MOVE || mt == EN_PASSANT || mt == PROMOTION) {
        Color color;
        PieceType piece;
        Square src_sq = move_source(mv);
        bool got = pos.get_piece_at(src_sq, color, piece);
        assert(got);
        if (piece == PAWN) {
            buf[bufidx++] = file_char(sq_file(src_sq));
        } else {
            buf[bufidx++] = piece_char(piece);
        }

        Bitboard occ = pos.get_bitboard(color, piece);
        if (!bboard::one_bit(occ)) {
            char ambiguity = 0; // 1 if some other piece is on the same rank, 2 if same file, 3 if both
            for (const Move& other_mv : legal_moves) {
                if (other_mv == mv) continue;

                Square other_src = move_source(other_mv);
                if (pos.get_bitboard(color, piece) & mask_square(other_src)) {
                    ambiguity |= sq_rank(src_sq) == sq_rank(other_src);
                    ambiguity |= (sq_file(src_sq) == sq_file(other_src)) << 1;
                }
            }

            switch (ambiguity) {
                case 0:
                case 2:
                    buf[bufidx++] = file_char(sq_file(src_sq));
                    break;
                case 3:
                    buf[bufidx++] = file_char(sq_file(src_sq));
                case 1:
                    buf[bufidx++] = '1' + sq_rank(src_sq);
                    break;
            }
        }

        // Add 'x' if is a capture
        bool is_capture = pos.get_all_bitboard() & mask_square(tgt_sq);
        buf[bufidx] = 'x';
        bufidx += is_capture;

        // Target square
        buf[bufidx++] = file_char(sq_file(tgt_sq));
        buf[bufidx++] = '1' + sq_rank(tgt_sq);

        // Promotion
        if (mt == PROMOTION) {
            buf[bufidx++] = '=';
            buf[bufidx++] = piece_char(move_promotion(mv));
        }

        // Final suffix
        if (checking) {
            buf[bufidx++] = '+';
        } else if (mating) {
            buf[bufidx++] = '#';
        }
    } else {
        bool queenside = !is_kingside_castle(tgt_sq);
        buf[3] = queenside * '-';
        buf[4] = queenside * 'O';
    }

    return string(buf);
}