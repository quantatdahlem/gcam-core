#' module_aglu_LB121.Carbon_LT
#'
#' Compute natural vegetation, managed land, and pasture carbon density, mature age, and yield.
#'
#' @param command API command to execute
#' @param ... other optional parameters, depending on command
#' @return Depends on \code{command}: either a vector of required inputs,
#' a vector of output names, or (if \code{command} is "MAKE") all
#' the generated outputs: \code{L121.CarbonContent_kgm2_R_LT_GLU}, \code{L121.Yield_kgm2_R_Past_GLU}. The corresponding file in the
#' original data system was \code{LB121.Carbon_LT.R} (aglu level1).
#' @details Combine Houghton et al. (1999 and similar) carbon density, mature age, and pasture yield data with
#' SAGE data to produce tables giving, for each GCAM region and land use type, data on these variables.
#' @importFrom assertthat assert_that
#' @importFrom dplyr filter mutate select
#' @importFrom tidyr gather spread
#' @importFrom stats weighted.mean
#' @author BBL April 2017
module_aglu_LB121.Carbon_LT <- function(command, ...) {
  if(command == driver.DECLARE_INPUTS) {
    return(c(FILE = "aglu/SAGE_LT",
             FILE = "aglu/Various_CarbonData_LTsage",
             "L120.LC_bm2_R_LT_Yh_GLU",
             "L120.LC_bm2_ctry_LTsage_GLU",
             "L120.LC_bm2_ctry_LTpast_GLU"))
  } else if(command == driver.DECLARE_OUTPUTS) {
    return(c("L121.CarbonContent_kgm2_R_LT_GLU",
             "L121.Yield_kgm2_R_Past_GLU"))
  } else if(command == driver.MAKE) {

    unit <- value <- Source <- variable <- pasture_yield <- GCAM_region_ID <-
        Land_Type <- GLU <- `mature age` <- Area_bm2 <- veg_c <- soil_c <-
        year <- NULL                    # silence package check.

    all_data <- list(...)[[1]]

    # Load required inputs
    SAGE_LT <- get_data(all_data, "aglu/SAGE_LT")
    Various_CarbonData_LTsage <- get_data(all_data, "aglu/Various_CarbonData_LTsage")
    L120.LC_bm2_R_LT_Yh_GLU <- get_data(all_data, "L120.LC_bm2_R_LT_Yh_GLU")
    L120.LC_bm2_ctry_LTsage_GLU <- get_data(all_data, "L120.LC_bm2_ctry_LTsage_GLU")
    L120.LC_bm2_ctry_LTpast_GLU <- get_data(all_data, "L120.LC_bm2_ctry_LTpast_GLU")

    # Convert characteristics by land type to correct units (kgC/m2)
    Various_CarbonData_LTsage %>%
      mutate(value = if_else(unit == "tC/ha", value * CONV_THA_KGM2, value)) %>%
      select(-Source, -unit) %>%
      spread(variable, value) ->
      L121.Various_CarbonData_LTsage

    # Need to get rid of the spaces in the table for matching
    L121.Various_CarbonData_LTsage$LT_SAGE <- gsub(" ", "", L121.Various_CarbonData_LTsage$LT_SAGE)

    # Match carbon contents and mature age by land cover, by SAGE land type
    L120.LC_bm2_ctry_LTsage_GLU %>%
      left_join(L121.Various_CarbonData_LTsage, by = "LT_SAGE") %>%
      select(-pasture_yield) %>%
      # Aggregate by GCAM region and GCAM land use type, using area-weighted mean
      group_by(GCAM_region_ID, Land_Type, GLU) %>%
      summarise(`mature age` = weighted.mean(`mature age`, Area_bm2),
                veg_c = weighted.mean(veg_c, Area_bm2),
                soil_c = weighted.mean(soil_c, Area_bm2)) %>%
      ungroup ->
      L121.CarbonContent_kgm2_R_LTnatveg_GLU

    # Urban land and cropland
    L120.LC_bm2_R_LT_Yh_GLU %>%
      spread(year, value) %>%
      filter(Land_Type %in% c("Cropland", "UrbanLand")) %>%
      select(GCAM_region_ID, Land_Type, GLU) %>%
      left_join_error_no_match(select(L121.Various_CarbonData_LTsage, -pasture_yield),
                               by = c("Land_Type" = "LT_SAGE")) ->
      L121.CarbonContent_kgm2_R_LTmgd_GLU

    # Pasture carbon contents
    L120.LC_bm2_ctry_LTpast_GLU %>%
      left_join_error_no_match(L121.Various_CarbonData_LTsage, by = "LT_SAGE") %>%
      # pasture lands may involve clearing; it is unclear to what extent this takes place but we'll use the carbon
      # contents of savanna as a maximum possible value for the vegetative carbon content of pasture lands
      mutate(veg_c = pmin(veg_c, L121.Various_CarbonData_LTsage$veg_c[L121.Various_CarbonData_LTsage$LT_SAGE == "Savanna"])) %>%

      # Aggregate by GCAM region and GCAM land use type, using area-weighted mean
      group_by(GCAM_region_ID, Land_Type, GLU) %>%
      summarise(`mature age` = weighted.mean(`mature age`, Area_bm2),
                veg_c = weighted.mean(veg_c, Area_bm2),
                soil_c = weighted.mean(soil_c, Area_bm2),
                pasture_yield = weighted.mean(pasture_yield, Area_bm2)) %>%
      ungroup ->
      L121.CarbonContent_kgm2_R_LTpast_GLU

    # Combine natural vegetation and managed land use tables
    bind_rows(L121.CarbonContent_kgm2_R_LTnatveg_GLU,
              L121.CarbonContent_kgm2_R_LTpast_GLU,
              L121.CarbonContent_kgm2_R_LTmgd_GLU) %>%
      select(GCAM_region_ID, Land_Type, GLU, `mature age`, soil_c, veg_c) %>%
      arrange(GCAM_region_ID, GLU, Land_Type) %>%
      add_title("Natural vegetation and managed land age and carbon density") %>%
      add_units("Years (mature age) and kgC/m2 (others)") %>%
      add_comments("From matching Houghton (1999) and SAGE data by GCAM region and land use type") %>%
      add_legacy_name("L121.CarbonContent_kgm2_R_LT_GLU") %>%
      add_precursors("aglu/SAGE_LT", "aglu/Various_CarbonData_LTsage",
                     "L120.LC_bm2_R_LT_Yh_GLU", "L120.LC_bm2_ctry_LTsage_GLU", "L120.LC_bm2_ctry_LTpast_GLU") ->
      L121.CarbonContent_kgm2_R_LT_GLU

    # Pasture yields are separate
    L121.CarbonContent_kgm2_R_LTpast_GLU %>%
      select(GCAM_region_ID, Land_Type, GLU, pasture_yield) %>%
      mutate(pasture_yield = pasture_yield / aglu.CCONTENT_CELLULOSE) %>%
      add_title("Pasture land age, carbon density, and yield") %>%
      add_units("Years (mature age) and kgC/m2 (others)") %>%
      add_comments("From matching Houghton (1999) and SAGE data by GCAM region and land use type") %>%
      add_legacy_name("L121.Yield_kgm2_R_Past_GLU") %>%
      add_precursors("aglu/SAGE_LT", "aglu/Various_CarbonData_LTsage",
                     "L120.LC_bm2_R_LT_Yh_GLU", "L120.LC_bm2_ctry_LTsage_GLU", "L120.LC_bm2_ctry_LTpast_GLU") ->
      L121.Yield_kgm2_R_Past_GLU

    return_data(L121.CarbonContent_kgm2_R_LT_GLU, L121.Yield_kgm2_R_Past_GLU)
  } else {
    stop("Unknown command")
  }
}
