#include <string.h>

#include "config.h"
#include "error.h"

typedef enum {
  OPTION_PLAYER_COUNT,
} OptionType;

void config_init(Config *config) {
  config->player_count = 1;
}

// Returns false if the option is not recognized.
static bool parse_short_option(const char *arg, OptionType *out) {
  switch (arg[0]) {
  case 'p':
    *out = OPTION_PLAYER_COUNT;
    return true;
  default:
    return false;
  }
}

// Returns false if the option is not recognized.
static bool parse_long_option(const char *arg, OptionType *out) {
  if (strcmp(arg, "player-count") == 0) {
    *out = OPTION_PLAYER_COUNT;
    return true;
  } else {
    return false;
  }
}

// Attempt to parse an unsigned int from the next argument. Only writes to out
// if argument was parsed successfully, returns whether parsing was successful.
static bool parse_uint(Config *cfg, ParseContext *ctx, unsigned int *out) {
  if (ctx->cursor == ctx->chunk_count)
    return false;

  const char *arg = ctx->chunks[ctx->cursor++];
  unsigned int acc = 0;
  for (int i = 0; i < strlen(arg); ++i) {
    char c = arg[i];
    if (c < '0' || c > '9') {
      return false;
    }

    unsigned int digit = c - 48;
    acc = acc * 10 + digit;
  }

  *out = acc;
  return true;
}

static bool parse_option_value(Config *cfg, ParseContext *ctx, OptionType opt) {
  switch (opt) {
  case OPTION_PLAYER_COUNT: {
    bool success = parse_uint(cfg, ctx, &cfg->player_count);
    if (!success) {
      report_error("expected integer value");
    }
    return success;
  }
  }
}

bool config_from_args(Config *cfg, int argc, const char **argv) {
  ParseContext ctx = {&argv[1], argc - 1, 0};

  while (ctx.cursor < ctx.chunk_count) {
    const char *arg = ctx.chunks[ctx.cursor++];
    OptionType option;

    // Identify the option.
    if (strncmp(arg, "--", 2) == 0) {
      if (!parse_long_option(&arg[2], &option)) {
        report_error("unknown argument '%s'", arg);
      }
    }
    else if (strncmp(arg, "-", 1) == 0) {
      // TODO: Handle flags.
      if (!parse_short_option(&arg[1], &option)) {
        report_error("unknown argument '%s'", arg);
      }
    }
    // If we encounter anything else in this position it is invalid.
    else {
      report_error("unexpected argument '%s'", arg);
      return false;
    }

    if (!parse_option_value(cfg, &ctx, option)) {
      return false;
    }
  }

  return true;
}
