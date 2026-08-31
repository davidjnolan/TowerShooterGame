// ©David Nolan, 2026

#pragma once

#include "CoreMinimal.h"

#include "EnemyWaveTypes.generated.h"



USTRUCT(BlueprintType)
struct TOWERSHOOTER_API FSpawnPointWeight
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
    FName SpawnPointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
    int32 Weight = 1;
};


USTRUCT(BlueprintType)
struct TOWERSHOOTER_API FEnemySpawnGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves", meta = (ClampMin = "0"))
	int32 Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
	TArray<FSpawnPointWeight> SpawnDistribution;
};


USTRUCT(BlueprintType)
struct TOWERSHOOTER_API FEnemyWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves")
	TArray<FEnemySpawnGroup> EnemyGroups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves", meta = (ClampMin = "0"))
	int32 MaxAliveEnemies = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Waves", meta = (ClampMin = "0.0"))
	float SpawnInterval = 0.5f;
};