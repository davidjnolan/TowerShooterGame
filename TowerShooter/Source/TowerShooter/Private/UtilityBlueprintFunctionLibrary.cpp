// ©David John Nolan, 2024

#include "UtilityBlueprintFunctionLibrary.h"

#include "GameplayTagsManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RenderTimer.h"
#include "RHI.h"

bool UUtilityBlueprintFunctionLibrary::BuildProjectFilePath(
	const FString& Directory,
	const FString& FileName,
	FString& OutFullPath
)
{
	// Clear the output first, so Blueprint/C++ callers do not keep an old path
	// if this function fails before creating a valid one.
	OutFullPath.Empty();

	// A file name is required. Without this, we do not know what file to write to.
	if (FileName.IsEmpty())
	{
		return false;
	}

	// Build a directory path relative to the project folder.
	//
	// Example:
	// ProjectDir = D:/TowerShooterGame/TowerShooter/
	// Directory  = Saved/Telemetry
	// Result     = D:/TowerShooterGame/TowerShooter/Saved/Telemetry
	const FString DirectoryPath = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectDir() / Directory
	);

	// Create the directory if it does not already exist.
	// The second argument, true, means "create the full directory tree".
	// So Saved/Telemetry will work even if neither folder exists yet.
	if (!IFileManager::Get().MakeDirectory(*DirectoryPath, true))
	{
		return false;
	}

	// Build the final absolute file path.
	//
	// Example:
	// D:/TowerShooterGame/TowerShooter/Saved/Telemetry/DamageEvents.csv
	OutFullPath = DirectoryPath / FileName;

	return true;
}


bool UUtilityBlueprintFunctionLibrary::TryRequestGameplayTag(
	const FName TagName,
	FGameplayTag& Tag
)
{
	// Request the Gameplay Tag from the Gameplay Tags Manager.
	// Passing false means "do not error if the tag does not exist".
	// If the tag is missing, Unreal returns an invalid Gameplay Tag instead.
	Tag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);

	// Return whether a valid tag was found.
	return Tag.IsValid();
}


bool UUtilityBlueprintFunctionLibrary::SaveStringToFile(
	const FString& Directory,
	const FString& FileName,
	const FString& FileContents,
	FString& OutFullPath
)
{
	// Build and validate the full path before trying to write the file.
	if (!BuildProjectFilePath(Directory, FileName, OutFullPath))
	{
		return false;
	}

	// Write the string contents to disk.
	// This replaces the existing file contents if the file already exists.
	return FFileHelper::SaveStringToFile(FileContents, *OutFullPath);
}


bool UUtilityBlueprintFunctionLibrary::AppendStringToFile(
	const FString& Directory,
	const FString& FileName,
	const FString& FileContents,
	FString& OutFullPath
)
{
	// Build and validate the full path before trying to append to the file.
	if (!BuildProjectFilePath(Directory, FileName, OutFullPath))
	{
		return false;
	}

	// Append the supplied string to the end of the file.
	// If the file does not exist, Unreal will create it automatically.
	return FFileHelper::SaveStringToFile(
		FileContents,
		*OutFullPath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		FILEWRITE_Append
	);
}


FString UUtilityBlueprintFunctionLibrary::GenerateRunID()
{
	// Get the local machine name.
	// This is useful when collecting reports from multiple playtest machines.
	const FString MachineName = FPlatformProcess::ComputerName();

	// Get the current local date and time.
	const FDateTime Now = FDateTime::Now();

	// Format the time so it is safe to use in file names.
	// Example result: 20260704_143215
	const FString Timestamp = Now.ToString(TEXT("%Y%m%d_%H%M%S"));

	// Combine machine name and timestamp into a single run ID.
	return FString::Printf(TEXT("%s_%s"), *MachineName, *Timestamp);
}


bool UUtilityBlueprintFunctionLibrary::TryParseBool(
	const FString& InString,
	bool& bParsedValue
)
{
	// Default the output to false. This ensures callers never receive
	// an uninitialised value if parsing fails.
	bParsedValue = false;

	// Remove any leading/trailing whitespace and convert to lower case
	// so comparisons are case-insensitive.
	const FString Normalised = InString.TrimStartAndEnd().ToLower();

	// Accepted true values.
	if (Normalised == TEXT("true") ||
		Normalised == TEXT("1") ||
		Normalised == TEXT("yes"))
	{
		bParsedValue = true;
		return true;
	}

	// Accepted false values.
	if (Normalised == TEXT("false") ||
		Normalised == TEXT("0") ||
		Normalised == TEXT("no"))
	{
		bParsedValue = false;
		return true;
	}

	// The string could not be interpreted as a boolean.
	return false;
}


bool UUtilityBlueprintFunctionLibrary::TryParseInt(
	const FString& InString,
	int32& ParsedValue
)
{
	ParsedValue = 0;

	const FString Normalised = InString.TrimStartAndEnd();

	// Reject empty strings.
	if (Normalised.IsEmpty())
	{
		return false;
	}

	// Verify every character is valid.
	int32 StartIndex = 0;

	if (Normalised.StartsWith(TEXT("-")))
	{
		StartIndex = 1;
	}

	for (int32 i = StartIndex; i < Normalised.Len(); ++i)
	{
		if (!FChar::IsDigit(Normalised[i]))
		{
			return false;
		}
	}

	ParsedValue = FCString::Atoi(*Normalised);

	return true;
}


bool UUtilityBlueprintFunctionLibrary::TryParseFloat(
	const FString& InString,
	float& ParsedValue
)
{
	ParsedValue = 0.0f;

	const FString Normalised = InString.TrimStartAndEnd();

	if (Normalised.IsEmpty())
	{
		return false;
	}

	bool bDecimalFound = false;
	int32 StartIndex = 0;

	if (Normalised.StartsWith(TEXT("-")))
	{
		StartIndex = 1;
	}

	for (int32 i = StartIndex; i < Normalised.Len(); ++i)
	{
		const TCHAR Character = Normalised[i];

		if (Character == '.')
		{
			if (bDecimalFound)
			{
				return false;
			}

			bDecimalFound = true;
			continue;
		}

		if (!FChar::IsDigit(Character))
		{
			return false;
		}
	}

	ParsedValue = FCString::Atof(*Normalised);

	return true;
}


bool UUtilityBlueprintFunctionLibrary::TryParseName(
	const FString& InString,
	FName& ParsedValue
)
{
	const FString Normalised = InString.TrimStartAndEnd();

	if (Normalised.IsEmpty())
	{
		ParsedValue = NAME_None;
		return false;
	}

	ParsedValue = FName(*Normalised);

	return true;
}


bool UUtilityBlueprintFunctionLibrary::TryParseVector(
	const FString& InString,
	FVector& ParsedValue
)
{
	// Default the output in case parsing fails.
	ParsedValue = FVector::ZeroVector;

	// Attempt to parse the string.
	return ParsedValue.InitFromString(InString);
}


bool UUtilityBlueprintFunctionLibrary::TryParseVector2D(
	const FString& InString,
	FVector2D& ParsedValue
)
{
	// Default the output in case parsing fails.
	ParsedValue = FVector2D::ZeroVector;

	// Attempt to parse the string.
	return ParsedValue.InitFromString(InString);
}


float UUtilityBlueprintFunctionLibrary::GetGameThreadTimeMs()
{
	// Convert Unreal's most recent Game Thread timing value to milliseconds.
	return FPlatformTime::ToMilliseconds(GGameThreadTime);
}


float UUtilityBlueprintFunctionLibrary::GetRenderThreadTimeMs()
{
	// Convert Unreal's most recent Render Thread timing value to milliseconds.
	return FPlatformTime::ToMilliseconds(GRenderThreadTime);
}


float UUtilityBlueprintFunctionLibrary::GetGPUFrameTimeMs()
{
	// Convert Unreal's most recent GPU frame timing value to milliseconds.
	return FPlatformTime::ToMilliseconds(GGPUFrameTime);
}


bool UUtilityBlueprintFunctionLibrary::IsPointInsideQuad2D(
	const FVector& Point,
	const FVector& TopLeft,
	const FVector& TopRight,
	const FVector& BottomRight,
	const FVector& BottomLeft)
{
	// Convert the world-space positions to 2D coordinates.
	// The test only operates on the XY plane, so Z is ignored.
	const FVector2D P(Point.X, Point.Y);
	const FVector2D TL(TopLeft.X, TopLeft.Y);
	const FVector2D TR(TopRight.X, TopRight.Y);
	const FVector2D BR(BottomRight.X, BottomRight.Y);
	const FVector2D BL(BottomLeft.X, BottomLeft.Y);

	// Calculate the 2D cross product of two vectors.
	//
	// For each edge of the quad, the sign of the cross product tells us
	// which side of that edge the point lies on.
	auto Cross2D = [](const FVector2D& A, const FVector2D& B)
	{
		return A.X * B.Y - A.Y * B.X;
	};

	// Test the point against each edge, moving clockwise around the quad:
	//
	// Top:    TL -> TR
	// Right:  TR -> BR
	// Bottom: BR -> BL
	// Left:   BL -> TL
	//
	// With the corners supplied in this order, a positive cross product
	// means the point lies outside that edge. Return immediately when this
	// happens so the remaining edges do not need to be tested.
	if (Cross2D(TR - TL, TL - P) > 0.0)
	return false;

	if (Cross2D(BR - TR, TR - P) > 0.0)
		return false;

	if (Cross2D(BL - BR, BR - P) > 0.0)
		return false;

	if (Cross2D(TL - BL, BL - P) > 0.0)
		return false;



	// The point did not fall outside any edge, so it is inside the quad.
	return true;
}


void UUtilityBlueprintFunctionLibrary::NoOp()
{
    // Intentionally does nothing.
}


TArray<FString> UUtilityBlueprintFunctionLibrary::GetFilesInDirectory(
    const FString& Directory,
    const FString& Extension)
{
    TArray<FString> FoundFiles;

    // Make sure the supplied directory is in a consistent format before
    // constructing the search pattern.
    FString NormalizedDirectory = Directory;
    FPaths::NormalizeDirectoryName(NormalizedDirectory);

    // Build the wildcard used by IFileManager.
    //
    // For example:
    //   Directory = "D:/Projects/TowerShooter/Snapshots"
    //   Extension = "json"
    //
    // produces:
    //   "D:/Projects/TowerShooter/Snapshots/*.json"
    //
    // If Extension is empty, search for all files instead.
    const FString SearchPattern = Extension.IsEmpty()
        ? FPaths::Combine(NormalizedDirectory, TEXT("*"))
        : FPaths::Combine(
            NormalizedDirectory,
            FString::Printf(TEXT("*.%s"), *Extension)
        );

    // FindFiles returns the names of matching files rather than their
    // complete paths.
    //
    // The first boolean means "include files".
    // The second boolean means "include directories".
    IFileManager::Get().FindFiles(
        FoundFiles,
        *SearchPattern,
        true,
        false
    );

    return FoundFiles;
}

