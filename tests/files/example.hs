{-# LANGUAGE LambdaCase, OverloadedStrings, RecordWildCards, TupleSections #-}
{-# OPTIONS_GHC -Wall -Wcompat #-}

{-
  SweetLine Haskell sample.
  Nested comments are part of the lexical rules:
  {- This inner comment should remain nested. -}
  The report and the GHC guide both note that pragmas and block comments are special.
  See https://www.haskell.org/onlinereport/haskell2010/haskellch2.html for the lexical rules.
-}

module Example.Syntax where

import qualified Data.List as List
import qualified Data.Map.Strict as Map
import qualified Data.Text as Text
import Data.Char (toLower)
import Data.Foldable (foldl')
import Prelude hiding (lookup)

infixl 6 <+>

newtype TaskId = TaskId Int deriving (Eq, Ord, Show)

data Priority
  = Low
  | Normal
  | High
  deriving (Eq, Ord, Show)

data Task = Task
  { taskId :: TaskId
  , taskTitle :: Text.Text
  , taskPriority :: Priority
  , taskTags :: [Text.Text]
  , taskDone :: Bool
  }
  deriving (Eq, Show)

type TaskIndex = Map.Map TaskId Task
type TaskReport = (Text.Text, [Task])

class Renderable a where
  render :: a -> Text.Text

instance Renderable Priority where
  render Low = "low"
  render Normal = "normal"
  render High = "high"

instance Renderable Task where
  render task@Task{..} =
    prettyTitle <> " [" <> priorityLabel taskPriority <> ", " <> statusText <> ", " <> tagList <> "]"
    where
      prettyTitle = Text.pack (map toLower (Text.unpack taskTitle))
      statusText = if taskDone then "done" else "open"
      tagList =
        if null taskTags
          then "none"
          else Text.intercalate ", " taskTags

renderTask :: Task -> Text.Text
renderTask = render

(<+>) :: Text.Text -> Text.Text -> Text.Text
(<+>) left right = left <> right

priorityLabel :: Priority -> Text.Text
priorityLabel p
  | p <= Low = "low"
  | p <= Normal = "normal"
  | otherwise = "high"

lookupTask :: TaskId -> TaskIndex -> Maybe Task
lookupTask key index = Map.lookup key index

buildIndex :: [Task] -> TaskIndex
buildIndex = foldl' insertTask Map.empty
  where
    insertTask index task = Map.insert (taskId task) task index

highPriorityTitles :: [Task] -> [Text.Text]
highPriorityTitles tasks =
  [ taskTitle task
  | task <- tasks
  , taskPriority task >= High
  , not (taskDone task)
  ]

describeLookup :: Maybe Task -> Text.Text
describeLookup result =
  case result of
    Just task -> "Found: " <> render task
    Nothing -> "Missing task"

describeTaskState :: Task -> Text.Text
describeTaskState task
  | taskDone task = "done"
  | otherwise = "pending"

renderSummary :: [Task] -> Text.Text
renderSummary tasks =
  let completed = [ taskTitle task | task <- tasks, taskDone task ]
      pending = [ taskTitle task | task <- tasks, not (taskDone task) ]
  in case completed of
    [] -> "No completed tasks."
    xs -> Text.intercalate ", " xs <> " / " <> Text.intercalate ", " pending

renderReport :: [Task] -> Text.Text
renderReport tasks =
  case List.sortOn taskTitle tasks of
    [] -> "No tasks."
    xs ->
      let openTasks = [ task | task <- xs, not (taskDone task) ]
          doneTasks = [ task | task <- xs, taskDone task ]
          labels = map taskTitle openTasks
      in Text.intercalate " | " (labels ++ [Text.pack (show (length doneTasks))])

renderTags :: [Text.Text] -> Text.Text
renderTags tags = Text.intercalate ", " tags

priorityCounts :: [Task] -> (Int, Int, Int)
priorityCounts tasks =
  let lowCount = length [ () | task <- tasks, taskPriority task <= Low ]
      normalCount = length [ () | task <- tasks, taskPriority task == Normal ]
      highCount = length [ () | task <- tasks, taskPriority task >= High ]
  in (lowCount, normalCount, highCount)

sectionExample :: Text.Text -> Text.Text
sectionExample text = ("[" <+>) text

escapedText :: Text.Text
escapedText = Text.pack "Line 1\\nLine 2\\t\\\"quoted\\\"\\\\backslash"

charExamples :: [Char]
charExamples = ['a', '\n', '\'', '\\']

applyIfReady :: Bool -> Text.Text -> Text.Text
applyIfReady ready message = if ready then message else "waiting"

balanced :: [Task] -> Text.Text
balanced tasks =
  if null tasks
    then "empty"
    else if length tasks > 1
      then "many"
      else "one"

sampleTasks :: [Task]
sampleTasks =
  [ Task { taskId = TaskId 1
         , taskTitle = Text.pack "Write docs"
         , taskPriority = High
         , taskTags = [Text.pack "docs", Text.pack "release"]
         , taskDone = False
         }
  , Task { taskId = TaskId 2
         , taskTitle = Text.pack "Fix parser"
         , taskPriority = Normal
         , taskTags = [Text.pack "core", Text.pack "bug"]
         , taskDone = True
         }
  , Task { taskId = TaskId 3
         , taskTitle = Text.pack "Ship release"
         , taskPriority = High
         , taskTags = [Text.pack "release"]
         , taskDone = False
         }
  ]

appMain :: [Task] -> IO ()
appMain tasks = do
  let index = buildIndex tasks
      missing = lookupTask (TaskId 99) index
  putStrLn (Text.unpack (renderReport tasks))
  putStrLn (Text.unpack (describeLookup missing))
  putStrLn (Text.unpack (renderTask (head tasks)))

main :: IO ()
main = appMain sampleTasks

taskBadge :: Task -> Text.Text
taskBadge task =
  case taskDone task of
    True -> "[x]"
    False -> "[ ]"

taskTitleLine :: Task -> Text.Text
taskTitleLine task =
  renderBadge <> " " <> render task
  where
    renderBadge = taskBadge task

{- Another nested comment follows.
   {- This comment is nested too. -}
   The operator section below is still part of the code.
-}

taskSummary :: Task -> Text.Text
taskSummary task =
  taskTitle task <+> " :: " <+> describeTaskState task
