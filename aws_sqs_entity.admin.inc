<?php

/**
 * @file
 * Admin functions for AWS SQS Entity module.
 */

/**
 * Form constructor for AWS SQS Entity settings.
 *
 * @see aws_sqs_entity_settings_validate()
 * @see aws_sqs_entity_settings_submit()
 */
function aws_sqs_entity_settings($form, &$form_state) {
  $form['aws_sqs_entity_queue_name'] = array(
    '#type' => 'textfield',
    '#title' => t('AWS SQS Entity queue name'),
    '#default_value' => variable_get('aws_sqs_entity_queue_name', ''),
    '#required' => TRUE,
    '#description' => t('AWS SQS Entity queue name.'),
  );

  $rules = variable_get('aws_sqs_entity_rules', array());
  $form['aws_sqs_entity_rules'] = array(
    '#type' => 'textarea',
    '#title' => t('AWS SQS Entity rules'),
    '#default_value' => aws_sqs_entity_settings_to_string($rules),
    '#description' => t('Configure the list of Entity types, bundles, and CRUD operations
   which should trigger an AWS SQS message. Each list item can be comma
   separated and/or on a new line, and should follow the pattern
   <code>ENTITY_TYPE:BUNDLE:CRUD_OPERATION</code> (where allowed Entity CRUD
   operations are: insert, update, delete).'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));

  return $form;
}

/**
 * Validate handler for AWS SQS Entity settings.
 *
 * @see aws_sqs_entity_settings()
 * @see aws_sqs_entity_settings_submit()
 */
function aws_sqs_entity_settings_validate($form, &$form_state) {
  $input = $form_state['values']['aws_sqs_entity_rules'];
  $rules = aws_sqs_entity_settings_rules_to_array($input);

  $info = entity_get_info();

  $messages = array();
  foreach ($rules as $type => $bundles) {
    $args['@type'] = $type;
    if (!array_key_exists($type, $info)) {
      $messages[] = t('@type is not a valid Entity type.', $args);
      continue;
    }
    foreach ($bundles as $bundle => $ops) {
      $args['@bundle'] = $bundle;
      if (!array_key_exists($bundle, $info[$type]['bundles'])) {
        $messages[] = t('@bundle is not a valid Bundle of Entity type @type.', $args);
        continue;
      }
      foreach ($ops as $op) {
        $args['@op'] = $op;
        if (!in_array($op, array('insert', 'update', 'delete'))) {
          $messages[] = t('@op is not a valid Entity CRUD operation.', $args);
          continue;
        }
      }
    }
  }

  if (!empty($messages)) {
    form_set_error('aws_sqs_entity_rules', theme('item_list', array('items' => $messages)));
  }

  $form_state['values']['aws_sqs_entity_rules_array'] = $rules;
}

/**
 * Submit handler for AWS SQS Entity settings.
 *
 * Note we don't use system_settings_form() because we want to easily override
 * the aws_sqs_entity_rules on form submission.
 *
 * @see aws_sqs_entity_settings()
 * @see aws_sqs_entity_settings_validate()
 */
function aws_sqs_entity_settings_submit($form, &$form_state) {
  variable_set('aws_sqs_entity_queue_name', $form_state['values']['aws_sqs_entity_queue_name']);

  $rules = $form_state['values']['aws_sqs_entity_rules_array'];
  variable_set('aws_sqs_entity_rules', $rules);
}

/**
 * Converts config text to the expected rules array format.
 *
 * @param string $input
 *   One or more config strings, separated by a new line or comma, each
 *   containing colon separated values following the pattern
 *   ENTITY:BUNDLE:CRUD_OPERATION, where allowed Entity CRUD operations are:
 *   - insert
 *   - update
 *   - delete
 *
 * @return array
 *   An array of AWS SQS Entity rules, in the format expected by
 *   \Drupal\aws_sqs_entity\Entity\CrudQueue::setRules().
 *
 * @see \Drupal\aws_sqs_entity\Entity\CrudQueue::setRules()
 * @see aws_sqs_entity_settings_to_string()
 */
function aws_sqs_entity_settings_rules_to_array($input) {
  $rules = array();

  // Allow for comma-separation or one on each line.
  $lines = preg_split( '/\R|,/', $input);
  foreach ($lines as $line) {
    list($type, $bundle, $op) = explode(':', trim($line));
    $rules[$type][$bundle][] = $op;
  }

  return $rules;
}

/**
 * Converts AWS SQS Entity rules array into a human-readable config text string.
 *
 * @param array $rules
 *   See $rules param for aws_sqs_entity_settings_rules_to_array().
 *
 * @return string
 *   See $input param for aws_sqs_entity_settings_rules_to_array().
 */
function aws_sqs_entity_settings_to_string($rules) {
  $strings = array();

  foreach ($rules as $type => $bundles) {
    foreach ($bundles as $bundle => $ops) {
      foreach ($ops as $op) {
        $strings[] = "$type:$bundle:$op";
      }
    }
  }

  return join("\n", $strings);
}