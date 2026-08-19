#include <fat.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "renderer.h"
#include "scoring.h"
#include "ui_utils.h"

static int  sd_mounted = 0;
static char app_name[64]     = "magnolia";
static char scores_path[160];
static char asset_buf[192];

int magnolia_init(const MagnoliaConfig *cfg) {
    int status = 0;

    if (cfg && cfg->app_name) {
        snprintf(app_name, sizeof(app_name), "%s", cfg->app_name);
    }

    /* Must precede any sd:/ open. Without it every texture load returns NULL
       and the game silently falls back to placeholder graphics. */
    sd_mounted = fatInitDefault() ? 1 : 0;
    if (!sd_mounted) status = -1;

    int rc = renderer_init();
    if (rc == -1) return -2;          /* no video: nothing further is useful */
    if (rc < 0) status = -3;          /* font missing, but we can still draw */

    if (cfg && cfg->overscan_pct >= 0) {
        ui_set_overscan_pct(cfg->overscan_pct);
    }

    int max = (cfg && cfg->max_scores > 0) ? cfg->max_scores : MAGNOLIA_MAX_SCORES;
    snprintf(scores_path, sizeof(scores_path), "sd:/apps/%s/scores.json", app_name);
    scoring_init(scores_path, max);

    return status;
}

void magnolia_shutdown(void) {
    renderer_shutdown();
}

int magnolia_sd_mounted(void)   { return sd_mounted; }
int magnolia_fonts_loaded(void) { return renderer_fonts_loaded(); }

const char *magnolia_asset_path(const char *leaf) {
    snprintf(asset_buf, sizeof(asset_buf), "sd:/apps/%s/%s", app_name, leaf ? leaf : "");
    return asset_buf;
}
