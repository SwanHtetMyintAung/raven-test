#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "cJSON.h"
#include "cvector.h"
#include "stdio.h"

typedef struct
{
    const char *word;
    const char *meaning;
} Example;

typedef struct {
    const char* kanji;
    const char* hiragana;
    const char* meaning;

    cvector_vector_type(Example) examples;
}Kanji;

typedef struct
{
    int day;

    cvector_vector_type(Kanji) kanjis;
} Day;

typedef struct
{
    int week;

    cvector_vector_type(Day) days;
} Week;

typedef struct
{
    cvector_vector_type(Week) weeks;

} SouMatome;

static void kanji_destroy(Kanji *kanji)
{
    if (!kanji)
        return;

    cvector_free(kanji->examples);
    kanji->examples = NULL;
}

static void day_destroy(Day *day)
{
    if (!day)
        return;

    for (size_t i = 0; i < cvector_size(day->kanjis); i++)
        kanji_destroy(&day->kanjis[i]);

    cvector_free(day->kanjis);
    day->kanjis = NULL;
}

static void week_destroy(Week *week)
{
    if (!week)
        return;

    for (size_t i = 0; i < cvector_size(week->days); i++)
        day_destroy(&week->days[i]);

    cvector_free(week->days);
    week->days = NULL;
}

static void soumatome_destroy(SouMatome *data)
{
    if (!data)
        return;

    for (size_t i = 0; i < cvector_size(data->weeks); i++)
        week_destroy(&data->weeks[i]);

    cvector_free(data->weeks);
    data->weeks = NULL;
}

static const char *json_get_string(
    const cJSON *object,
    const char *key)
{
    const cJSON *item =
        cJSON_GetObjectItemCaseSensitive(object, key);

    if (!cJSON_IsString(item))
        return NULL;

    return cJSON_GetStringValue(item);
}

static bool json_get_int(
    const cJSON *object,
    const char *key,
    int *result)
{
    const cJSON *item =
        cJSON_GetObjectItemCaseSensitive(object, key);

    if (!cJSON_IsNumber(item))
        return false;

    double value = cJSON_GetNumberValue(item);

    if (!isfinite(value) ||
        value < INT_MIN ||
        value > INT_MAX ||
        floor(value) != value)
    {
        return false;
    }

    *result = (int)value;
    return true;
}

static bool parse_examples(
    const cJSON *json,
    Kanji *kanji)
{
    if (!json)
        return true; // Examples are optional.

    if (!cJSON_IsArray(json))
        return false;

    const cJSON *item;

    cJSON_ArrayForEach(item, json)
    {
        if (!cJSON_IsObject(item))
            return false;

        Example example = {0};

        example.word =
            json_get_string(item, "word");

        example.meaning =
            json_get_string(item, "meaning");

        if (!example.word || !example.meaning)
            return false;

        cvector_push_back(kanji->examples, example);
    }

    return true;
}

static bool parse_kanji(
    const cJSON *json,
    Kanji *result)
{
    if (!cJSON_IsObject(json))
        return false;

    Kanji kanji = {0};

    kanji.kanji =
        json_get_string(json, "kanji");

    kanji.hiragana =
        json_get_string(json, "hiragana");

    kanji.meaning =
        json_get_string(json, "meaning");

    if (!kanji.kanji ||
        !kanji.hiragana ||
        !kanji.meaning)
    {
        goto error;
    }

    const cJSON *examples =
        cJSON_GetObjectItemCaseSensitive(json, "examples");

    if (!parse_examples(examples, &kanji))
        goto error;

    *result = kanji;

    return true;

error:
    kanji_destroy(&kanji);
    return false;
}

static bool parse_day(
    const cJSON *json,
    Day *result)
{
    if (!cJSON_IsObject(json))
        return false;

    Day day = {0};

    if (!json_get_int(json, "day", &day.day))
        goto error;

    const cJSON *kanjis =
        cJSON_GetObjectItemCaseSensitive(json, "kanjis");

    if (!cJSON_IsArray(kanjis))
        goto error;

    const cJSON *item;

    cJSON_ArrayForEach(item, kanjis)
    {
        Kanji kanji = {0};

        if (!parse_kanji(item, &kanji))
            goto error;

        cvector_push_back(day.kanjis, kanji);
    }

    *result = day;

    return true;

error:
    day_destroy(&day);
    return false;
}

static bool parse_week(
    const cJSON *json,
    Week *result)
{
    if (!cJSON_IsObject(json))
        return false;

    Week week = {0};

    if (!json_get_int(json, "week", &week.week))
        goto error;

    const cJSON *days =
        cJSON_GetObjectItemCaseSensitive(json, "days");

    if (!cJSON_IsArray(days))
        goto error;

    const cJSON *item;

    cJSON_ArrayForEach(item, days)
    {
        Day day = {0};

        if (!parse_day(item, &day))
            goto error;

        cvector_push_back(week.days, day);
    }

    *result = week;

    return true;

error:
    week_destroy(&week);
    return false;
}

static bool parse_soumatome(
    const cJSON *json,
    SouMatome *result)
{
    if (!cJSON_IsObject(json))
        return false;

    SouMatome data = {0};

    const cJSON *weeks =
        cJSON_GetObjectItemCaseSensitive(json, "weeks");

    if (!cJSON_IsArray(weeks))
        goto error;

    const cJSON *item;

    cJSON_ArrayForEach(item, weeks)
    {
        Week week = {0};

        if (!parse_week(item, &week))
            goto error;

        cvector_push_back(data.weeks, week);
    }

    *result = data;

    return true;

error:
    soumatome_destroy(&data);
    return false;
}

static char* parseJson(const char* filePath)
{
    printf("getting in\n");
    if (filePath == NULL) {
        printf("filepath is null\n");
        return NULL;
    }

    FILE* file = fopen(filePath, "rb");

    if (file == NULL)
    {
        perror("fopen failed");
        printf("file pointer is null\n");
        return NULL;
    }

    // Get file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        printf("something wrong at fseek\n");
        return NULL;
    }

    long fileSize = ftell(file);

    if (fileSize < 0) {
        fclose(file);
        printf("something wrong at ftell\n");
        return NULL;
    }

    rewind(file);

    // Allocate memory for file contents
    char* jsonString = malloc((size_t)fileSize + 1);

    if (jsonString == NULL) {
        fclose(file);
        printf("something wrong at malloc\n");
        return NULL;
    }

    // Read file
    size_t bytesRead = fread(jsonString, 1, (size_t)fileSize, file);

    fclose(file);

    if (bytesRead != (size_t)fileSize) {
        free(jsonString);
        printf("something wrong at bytes read\n");
        return NULL;
    }

    jsonString[bytesRead] = '\0';

    // Validate JSON using cJSON
    cJSON* json = cJSON_Parse(jsonString);

    if (json == NULL) {
        free(jsonString);
        printf("something wrong at json parse\n");

        return NULL;
    }

    cJSON_Delete(json);

    return jsonString;
}
