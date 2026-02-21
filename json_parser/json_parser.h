/******************************************************************************
 * json_parser.h
 *
 * A full-featured JSON parser, serializer, and DOM API.
 *
 * Usage:
 *   1. Parse a string: json_value_t* root = json_parse(input, &error);
 *   2. Use the API to access data: json_get_string(root, "key");
 *   3. Serialize back to JSON: json_serialize(root, pretty, output_file);
 *   4. Free all memory: json_free(root);
 ******************************************************************************/

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stddef.h>   /* size_t */
#include <stdio.h>    /* FILE* */

/* ----------------------------------------------------------------------------
   Public error information
---------------------------------------------------------------------------- */
typedef struct json_error {
    const char *message;   /* Human-readable error description */
    int line;              /* Line number where error occurred (1-based) */
    int column;            /* Column number (1-based) */
} json_error_t;

/* ----------------------------------------------------------------------------
   JSON value type enumeration
---------------------------------------------------------------------------- */
typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type_t;

/* ----------------------------------------------------------------------------
   Forward declaration of the main JSON value structure.
   The actual definition is opaque to the public API to encourage using
   the accessor functions.
---------------------------------------------------------------------------- */
typedef struct json_value json_value_t;

/* ----------------------------------------------------------------------------
   Parsing entry point
   Returns a pointer to the root value, or NULL on error.
   If error is not NULL, it will be filled with details.
---------------------------------------------------------------------------- */
json_value_t* json_parse(const char *input, json_error_t *error);

/* ----------------------------------------------------------------------------
   Serialization
   - json_serialize: writes minified JSON to a file.
   - json_pretty_print: writes human-readable JSON with indentation.
---------------------------------------------------------------------------- */
void json_serialize(const json_value_t *value, FILE *out);
void json_pretty_print(const json_value_t *value, FILE *out);

/* ----------------------------------------------------------------------------
   DOM-style accessor functions
   All return NULL if the requested element does not exist or type mismatches.
   Strings returned are owned by the json_value_t and must not be freed.
---------------------------------------------------------------------------- */
json_type_t json_type(const json_value_t *value);
int          json_is_null(const json_value_t *value);
int          json_bool_value(const json_value_t *value);
double       json_number_value(const json_value_t *value);
const char*  json_string_value(const json_value_t *value);

/* Array access */
size_t       json_array_length(const json_value_t *value);
json_value_t* json_array_get(const json_value_t *value, size_t index);

/* Object access */
size_t       json_object_size(const json_value_t *value);
const char*  json_object_key(const json_value_t *value, size_t index);
json_value_t* json_object_get(const json_value_t *value, const char *key);

/* ----------------------------------------------------------------------------
   Memory management
   Frees the entire tree allocated during parsing.
---------------------------------------------------------------------------- */
void json_free(json_value_t *value);

#endif /* JSON_PARSER_H */

/******************************************************************************
 * json_parser.c
 *
 * Implementation of the JSON parser.
 *
 * Internal design:
 *   - Lexer: breaks input into tokens with line/column tracking.
 *   - Parser: recursive descent, constructs an AST using an arena allocator.
 *   - Arena: bump allocator for all nodes and strings – fast and easy cleanup.
 *   - Errors: stored in a context and reported through json_error_t.
 *
 * All public functions are documented in the header.
 ******************************************************************************/
