// ©David John Nolan, 2024

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "UtilityBlueprintFunctionLibrary.generated.h"

UCLASS()
class TOWERSHOOTER_API UUtilityBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
private:
	/**
	 * Builds a full file path from a project-relative directory and file name.
	 *
	 * The directory is relative to the project folder.
	 * Missing directories will be created automatically.
	 *
	 * Example:
	 * Directory: Saved/Telemetry
	 * FileName: DamageEvents.csv
	 *
	 * Result:
	 * <ProjectFolder>/Saved/Telemetry/DamageEvents.csv
	 *
	 * Returns true if a valid file path was created.
	 * OutFullPath contains the absolute path to the file.
	 */
	static bool BuildProjectFilePath(
		const FString& Directory,
		const FString& FileName,
		FString& OutFullPath
	);


public:

	/**
	 * Attempts to retrieve a Gameplay Tag by name.
	 *
	 * This is useful when the tag name is only known at runtime, such as when
	 * loading data from a Data Table, CSV, or other external source.
	 *
	 * Returns true if the requested Gameplay Tag exists in the Gameplay Tag
	 * Manager. If successful, the output Tag parameter will contain the
	 * requested tag. If the tag does not exist, the output tag will be invalid.
	 */
	UFUNCTION(BlueprintPure, Category = "GameplayTags")
	static bool TryRequestGameplayTag(FName TagName, FGameplayTag& Tag);

	
	/**
	 * Saves a string to a text file, replacing any existing contents.
	 *
	 * The directory is relative to the project folder.
	 * Missing directories will be created automatically.
	 *
	 * Returns true if the file was written successfully.
	 * OutFullPath contains the absolute path to the file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|File")
	static bool SaveStringToFile(
		const FString& Directory,
		const FString& FileName,
		const FString& FileContents,
		FString& OutFullPath
	);

	/**
	 * Appends a string to the end of a text file.
	 *
	 * The directory is relative to the project folder.
	 * Missing directories will be created automatically.
	 *
	 * If the file does not already exist, it will be created.
	 *
	 * Returns true if the string was appended successfully.
	 * OutFullPath contains the absolute path to the file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|File")
	static bool AppendStringToFile(
		const FString& Directory,
		const FString& FileName,
		const FString& FileContents,
		FString& OutFullPath
	);


	/**
	 * Generates a unique-ish run ID for a playtest or telemetry report.
	 *
	 * The ID combines the local machine name with the current date and time.
	 *
	 * Example:
	 * DESKTOP-ABC123_20260704_143215
	 *
	 * This is useful for naming report files and identifying which machine
	 * generated a particular playtest run.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Telemetry")
	static FString GenerateRunID();


	/**
	 * Attempts to parse a string as a boolean.
	 *
	 * Accepted true values (case-insensitive):
	 * - "True"
	 * - "1"
	 * - "Yes"
	 *
	 * Accepted false values (case-insensitive):
	 * - "False"
	 * - "0"
	 * - "No"
	 *
	 * Returns true if the string was successfully parsed.
	 * bParsedValue contains the parsed boolean value.
	 *
	 * Returns false if the string could not be interpreted as a boolean.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseBool(
		const FString& InString,
		bool& bParsedValue
	);

	/**
	 * Attempts to parse a string as an integer.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseInt(
		const FString& InString,
		int32& ParsedValue
	);

	/**
	 * Attempts to parse a string as a floating point number.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseFloat(
		const FString& InString,
		float& ParsedValue
	);

	/**
	 * Attempts to parse a string as a Name.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseName(
		const FString& InString,
		FName& ParsedValue
	);

	/**
	 * Attempts to parse a string as an FVector.
	 *
	 * Supported formats include:
	 * - X=100 Y=200 Z=300
	 *
	 * Returns true if the string was successfully parsed.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseVector(
		const FString& InString,
		FVector& ParsedValue
	);

	/**
	 * Attempts to parse a string as an FVector2D.
	 *
	 * Supported formats include:
	 * - X=100 Y=200
	 *
	 * Returns true if the string was successfully parsed.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Conversion")
	static bool TryParseVector2D(
		const FString& InString,
		FVector2D& ParsedValue
	);

	/**
	 * Returns the time spent on the Game Thread during the most recently
	 * measured frame, in milliseconds.
	 *
	 * This is useful for performance benchmarking and telemetry without
	 * relying on console stat displays.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Performance")
	static float GetGameThreadTimeMs();


	/**
	 * Returns the time spent on the Render Thread during the most recently
	 * measured frame, in milliseconds.
	 *
	 * This is useful for performance benchmarking and telemetry without
	 * relying on console stat displays.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Performance")
	static float GetRenderThreadTimeMs();


	/**
	 * Returns the GPU frame time from Unreal's most recently available
	 * GPU timing measurement, in milliseconds.
	 *
	 * This is useful for performance benchmarking and telemetry without
	 * relying on console stat displays.
	 */
	UFUNCTION(BlueprintPure, Category = "Utilities|Performance")
	static float GetGPUFrameTimeMs();


	/**
	 * Tests whether a world-space point lies inside a four-sided area
	 * when viewed in the XY plane.
	 *
	 * The quad is defined by four corners supplied in clockwise order:
	 * TopLeft, TopRight, BottomRight, BottomLeft.
	 *
	 * The Z component of all positions is ignored.
	 *
	 * Returns true if the point lies inside the quad or directly on one
	 * of its edges.
	 */
	UFUNCTION(BlueprintPure, Category = "Math")
	static bool IsPointInsideQuad2D(
		const FVector& Point,
		const FVector& TopLeft,
		const FVector& TopRight,
		const FVector& BottomRight,
		const FVector& BottomLeft
	);

	/**
	 * Performs no operation.
	 *
	 * Intended as an explicit endpoint or marker in Blueprint execution flow,
	 * making intentionally unused execution paths easier to read.
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities")
	static void NoOp();

	/**
	 * Returns the filenames of files in a directory that match the supplied extension.
	 *
	 * @param Directory     Absolute directory to search.
	 * @param Extension     File extension to search for, without the leading dot
	 *                      (for example "json"). Leave empty to return all files.
	 *
	 * @return              An array containing filenames only, not full paths.
	 *                      Returns an empty array if the directory does not exist
	 *                      or no matching files are found.
	 */
	UFUNCTION(BlueprintPure, Category = "File Utilities")
	static TArray<FString> GetFilesInDirectory(
		const FString& Directory,
		const FString& Extension
	);


};
